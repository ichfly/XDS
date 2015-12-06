#include "Kernel.h"

#include <stdio.h>

#include "Bootloader.h"

//they are in mem in that order fs loader pm sm pix



//tools
static u32 Read32revers(uint8_t p[4])
{
    u32 temp = p[3] | p[2] << 8 | p[1] << 16 | p[0] << 24;
    return temp;
}

static u32 Read32(uint8_t p[4])
{
    u32 temp = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
    return temp;
}
static u64 Read64(uint8_t p[8])
{
    u64 temp = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24 | (u64)(p[4]) << 32 | (u64)(p[5]) << 40 | (u64)(p[6]) << 48 | (u64)(p[7]) << 56;
    return temp;
}
static u16 Read16(uint8_t p[2])
{
    u16 temp = p[0] | p[1] << 8;
    return temp;
}
static u32 AlignPage(u32 in)
{
    return ((in + 0xFFF) / 0x1000) * 0x1000;
}
static u32 GetDecompressedSize(u8* compressed, u32 compressedsize)
{
    u8* footer = compressed + compressedsize - 8;

    u32 originalbottom = Read32(footer + 4);
    return originalbottom + compressedsize;
}

static int Decompress(u8* compressed, u32 compressedsize, u8* decompressed, u32 decompressedsize)
{

    u8* footer = compressed + compressedsize - 8;
    u32 buffertopandbottom = Read32(footer + 0);
    u32 i, j;
    u32 out = decompressedsize;
    u32 index = compressedsize - ((buffertopandbottom >> 24) & 0xFF);
    u32 segmentoffset;
    u32 segmentsize;
    u8 control;
    u32 stopindex = compressedsize - (buffertopandbottom & 0xFFFFFF);

    memset(decompressed, 0, decompressedsize);
    memcpy(decompressed, compressed, compressedsize);

    while (index > stopindex) {
        control = compressed[--index];

        for (i = 0; i<8; i++) {
            if (index <= stopindex)
                break;
            if (index <= 0)
                break;
            if (out <= 0)
                break;

            if (control & 0x80) {
                if (index < 2) {
                    fprintf(stderr, "Error, compression out of bounds");
                    goto clean;
                }

                index -= 2;

                segmentoffset = compressed[index] | (compressed[index + 1] << 8);
                segmentsize = ((segmentoffset >> 12) & 15) + 3;
                segmentoffset &= 0x0FFF;
                segmentoffset += 2;

                if (out < segmentsize) {
                    fprintf(stderr, "Error, compression out of bounds");
                    goto clean;
                }

                for (j = 0; j<segmentsize; j++) {
                    u8 data;

                    if (out + segmentoffset >= decompressedsize) {
                        fprintf(stderr, "Error, compression out of bounds");
                        goto clean;
                    }

                    data = decompressed[out + segmentoffset];
                    decompressed[--out] = data;
                }
            }
            else {
                if (out < 1) {
                    fprintf(stderr, "Error, compression out of bounds");
                    goto clean;
                }
                decompressed[--out] = compressed[--index];
            }
            control <<= 1;
        }
    }

    return 1;
clean:
    return 0;
}

KProcess* Boot_LoadFileFast(FILE* fd, u32 offset, u32* out_offset, KKernel * Kernel)
{

    if (fseek(fd, offset, SEEK_SET) != 0)
    {
        XDSERROR("failed to seek.");
        return NULL;
    }

    exheader_header ex;
    ctr_ncchheader loader_h;
    u32 ncch_off = 0;

    // Read header.
    if (fread(&loader_h, sizeof(loader_h), 1, fd) != 1) {
        XDSERROR("failed to read header.");
        return NULL;
    }

    // Load NCCH
    if (memcmp(&loader_h.magic, "NCCH", 4) != 0) {
        XDSERROR("invalid magic.. wrong file?");
        return NULL;
    }

    // Read Exheader.
    if (fread(&ex, sizeof(ex), 1, fd) != 1) {
        XDSERROR("failed to read exheader.");
        return NULL;
    }

    bool is_compressed = ex.codesetinfo.flags.flag & 1;
    char namereal[9];
    strncpy(namereal, (char*)ex.codesetinfo.name, 9);

    // Read ExeFS.
    u32 exefs_off = Read32(loader_h.exefsoffset) * 0x200;
    u32 exefs_sz = Read32(loader_h.exefssize) * 0x200;

    if (fseek(fd, exefs_off + ncch_off + offset, SEEK_SET) != 0)
    {
        XDSERROR("failed to seek.");
        return NULL;
    }

    exefs_header eh;
    if (fread(&eh, sizeof(eh), 1, fd) != 1) {
        XDSERROR("failed to read ExeFS header.");
        return NULL;
    }

    for (u32 i = 0; i < 8; i++) {
        u32 sec_size = Read32(eh.section[i].size);
        u32 sec_off = Read32(eh.section[i].offset);

        if (sec_size == 0)
            continue;

        eh.section[i].name[7] = '\0';

        if (strcmp((char*)eh.section[i].name, ".code") == 0) {
            sec_off += exefs_off + sizeof(eh);
            if (fseek(fd, sec_off + ncch_off + offset, SEEK_SET) != 0)
            {
                XDSERROR("failed to seek.");
                return NULL;
            }

            u8* sec = (u8*)malloc(AlignPage(sec_size));
            if (sec == NULL) {
                XDSERROR("section malloc failed.");
                return NULL;
            }

            if (fread(sec, sec_size, 1, fd) != 1) {
                XDSERROR("section fread failed.");
                free(sec);
                return NULL;
            }


            // Decompress first section if flag set.
            if (i == 0 && is_compressed) {
                u32 dec_size = GetDecompressedSize(sec, sec_size);
                u8* dec = (u8*)malloc(AlignPage(dec_size));

                if (!dec) {
                    XDSERROR("decompressed data block allocation failed.");
                    free(sec);
                    return NULL;
                }

                u32 firmexpected = Read32(ex.codesetinfo.text.codesize) + Read32(ex.codesetinfo.ro.codesize) + Read32(ex.codesetinfo.data.codesize);

                if (Decompress(sec, sec_size, dec, dec_size) == 0) {
                    XDSERROR("section decompression failed.");
                    free(sec);
                    free(dec);
                    return NULL;
                }

                /*FILE * pFile;
                pFile = fopen("code.code", "wb");
                if (pFile != NULL)
                {
                    fwrite(dec, 1, dec_size, pFile);
                    fclose(pFile);
                }*/

                free(sec);
                sec = dec;
                sec_size = dec_size;
            }

            // Load .code section.
            u32 realcodesize = Read32(ex.codesetinfo.text.codesize);
            u32 codesize = AlignPage(realcodesize);
            u8* code = (u8*)malloc(codesize);
            if (!code) {
                XDSERROR("text data block allocation failed.");
                free(sec);
                return NULL;
            }
            memset(code,0, codesize);
            memcpy(code, sec, realcodesize);

            u32 realrodatasize = Read32(ex.codesetinfo.ro.codesize);
            u32 rodatasize = AlignPage(realrodatasize);
            u8* rodata = (u8*)malloc(rodatasize);
            if (!rodata) {
                XDSERROR("rodata data block allocation failed.");
                free(code);
                free(sec);
                return NULL;
            }
            memset(rodata, 0, rodatasize);
            memcpy(rodata, sec + realcodesize, realrodatasize);

            u32 realdatasize = Read32(ex.codesetinfo.data.codesize);
            u32 datasize = AlignPage(realdatasize);
            u8* data = (u8*)malloc(datasize);
            if (!data) {
                XDSERROR("data data block allocation failed.");
                free(code);
                free(sec);
                free(rodata);
                return NULL;
            }
            memset(data, 0, datasize);
            memcpy(data, sec + realcodesize + realrodatasize, realdatasize);

            KCodeSet* Codeset = new KCodeSet(
                code, codesize / 0x1000,
                rodata, rodatasize/0x1000,
                data, datasize / 0x1000,
                AlignPage(Read32(ex.codesetinfo.bsssize)) / 0x1000,
                Read64(loader_h.programid),
                namereal
                );

            KProcess* process = new KProcess(Codeset, 28, (u32*)ex.arm11kernelcaps.descriptors, Kernel,true);

            //Start the process

            //map stack
            u32 unused;
            u32 stacksize = Read32(ex.codesetinfo.stacksize);
            process->getMemoryMap()->ControlMemory(&unused, 0x10000000 - stacksize, 0, stacksize, OPERATION_COMMIT, PERMISSION_RW);

            KThread * thread = new KThread(SERVICECORE,process);
            u32 startaddr = (process->m_exheader_flags & (1 << 12)) ? 0x14000000 : 0x00100000;
            thread->m_context.reg_15 = startaddr;
            thread->m_context.pc = startaddr;
            thread->m_context.sp = 0x10000000;
			thread->m_context.cpsr = 0x1F;
            process->AddThread(thread);

            free(rodata);
            free(data);
            free(code);
            free(sec);

            *out_offset = ftell(fd);

            //no need to register them with fs and sm the pm dose that on its own core stuff always dose that correctly
            return process;
        }
    }
    return NULL;
}
FILE* openapp(u32 titlehigh, u32 titlelow) //used by pm
{
    for (u32 i = 0; i < 0x1000; i++) //TODO search for the correct tmd that needs to be impr that is also only a hack normaly the data is loaded from .firm not from the FS
    {
        char string[0x100];
#if EMU_PLATFORM == PLATFORM_WINDOWS
		sprintf_s(string, 0x100, "./NAND/title/%08X/%08X/content/%08x.tmd", titlehigh, titlelow, i);
#else
        snprintf(string, 0x100, "./NAND/title/%08X/%08X/content/%08x.tmd", titlehigh, titlelow, i);
#endif
        FILE* fd = fopen(string, "rb");
        if (fd != NULL)
        {
            u8 temp[4];
            if (fread(temp, 4, 1, fd) != 1)
            {
                XDSERROR("reading tmd Signature Type");
                return NULL;
            }
            u32 Signature_Type = Read32revers(temp);
            u32 y;
            switch (Signature_Type)
            {
            case 0x010000:
                y = 0x240;
                break;
            case 0x010001:
                y = 0x140;
                break;
            case 0x010002:
                y = 0x80;
                break;
            case 0x010003:
                y = 0x240;
                break;
            case 0x010004:
                y = 0x140;
                break;
            case 0x010005:
                y = 0x80;
                break;
            default:
                LOG("unknown Signature Type fallback");
                y = 0x140;
                break;
            }
            if (fseek(fd, y + 0x9C4, SEEK_SET) != 0)
            {
                XDSERROR("reading tmd Signature Type");
                return NULL;
            }
            if (fread(temp, 4, 1, fd) != 1)
            {
                XDSERROR("reading tmd Signature Type");
                return NULL;
            }
            u32 index = Read32revers(temp);
            fclose(fd);

#if EMU_PLATFORM == PLATFORM_WINDOWS
			sprintf_s(string, 0x100, "./NAND/title/%08x/%08x/content/%08x.app", titlehigh, titlelow, index);
#else
			snprintf(string, 0x100, "./NAND/title/%08x/%08x/content/%08x.app", titlehigh, titlelow, index);
#endif
            fd = fopen(string, "rb");
            if (fd == NULL)
            {
                XDSERROR("opening the container %s", string);
                return NULL;
            }
            return fd;
        }
    }
    return NULL;
}

s64 FindRomFSOffset(FILE* fd, char* name, u64 &out_size, u8* hash_out)
{
	if (fd == NULL)
	{
		return -1;
	}
	fseek(fd, 0, SEEK_SET);


	//open the container
	exheader_header ex;
	ctr_ncchheader loader_h;
	u32 ncch_off = 0;

	// Read header.
	if (fread(&loader_h, sizeof(loader_h), 1, fd) != 1) {
		XDSERROR("failed to read header.");
		return -1;
	}
	// Load NCCH
	if (memcmp(&loader_h.magic, "NCCH", 4) != 0) {
		XDSERROR("invalid magic.. wrong file?");
		return -1;
	}

	// Read Exheader.
	if (fread(&ex, sizeof(ex), 1, fd) != 1) {
		XDSERROR("failed to read exheader.");
		return -1;
	}

	// Read ExeFS.
	u32 exefs_off = Read32(loader_h.romfsoffset) * 0x200;
	out_size = Read32(loader_h.romfssize) * 0x200;

	return exefs_off;
}


s64 FindTableOffset(FILE* fd,char* name,u64 &out_size, u8* hash_out)
{
	if (fd == NULL)
	{
		return -1;
	}
	fseek(fd, 0, SEEK_SET);


	//open the container
	exheader_header ex;
	ctr_ncchheader loader_h;
	u32 ncch_off = 0;

	// Read header.
	if (fread(&loader_h, sizeof(loader_h), 1, fd) != 1) {
		XDSERROR("failed to read header.");
		return -1;
	}
	// Load NCCH
	if (memcmp(&loader_h.magic, "NCCH", 4) != 0) {
		XDSERROR("invalid magic.. wrong file?");
		return -1;
	}

	// Read Exheader.
	if (fread(&ex, sizeof(ex), 1, fd) != 1) {
		XDSERROR("failed to read exheader.");
		return -1;
	}

	// Read ExeFS.
	u32 exefs_off = Read32(loader_h.exefsoffset) * 0x200;
	u32 exefs_sz = Read32(loader_h.exefssize) * 0x200;

	if (fseek(fd, exefs_off + ncch_off, SEEK_SET) != 0)
	{
		XDSERROR("failed to seek.");
		return -2;
	}

	exefs_header eh;
	if (fread(&eh, sizeof(eh), 1, fd) != 1) {
		XDSERROR("failed to read ExeFS header.");
		return -1;
	}

	for (u32 i = 0; i < 8; i++) {
		u32 sec_size = Read32(eh.section[i].size);
		u32 sec_off = Read32(eh.section[i].offset);
		out_size = sec_size;
		if (sec_size == 0)
			continue;

		eh.section[i].name[7] = '\0';

		if (hash_out != NULL)
		{
			memcpy(hash_out, eh.hashes[7-i], 0x20);
		}

		//sec_off = 0;

		if (strcmp((char*)eh.section[i].name, name) == 0) {
			return exefs_off + sizeof(eh) + sec_off;
		}
	}
	XDSERROR("finding section");
	return -1;
}

int Boot(KKernel* kernel)
{
    FILE* fd = openapp(0x00040138, 0x00000002);//this is firm
    //open the firm to extrect the core modules
	u64 temp;
	s64 sec_off = FindTableOffset(fd, ".firm", temp, NULL);
    u32 out_offset;
	if (sec_off > 0)
	{
		KProcess* process = Boot_LoadFileFast(fd, sec_off + 0x200, &out_offset, kernel);//The first is most likely the NCCH container but that may change I don't know how to detect the correct container so I just do it by a static offset TODO
		for (int j = 0; j < 4; j++) //boot all 5
		{
			process = Boot_LoadFileFast(fd, (out_offset + 0x1FF)&~0x1FF, &out_offset, kernel);
		}
		fclose(fd);
		return 0; //it worked
	}
    fclose(fd);
    XDSERROR("finding .firm section");
    return -1;
    XDSERROR("finding firm");
    return -1;

}
