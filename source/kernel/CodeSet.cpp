#include "Kernel.h"

static u32 Read32(uint8_t p[4])
{
	u32 temp = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
	return temp;
}
static u16 Read16(uint8_t p[2])
{
	u16 temp = p[0] | p[1] << 8;
	return temp;
}

static u32 TranslateAddr(u32 addr, struct _3DSX_LoadInfo *d, u32* offsets)
{
	if (addr < offsets[0])
		return d->segAddrs[0] + addr;
	if (addr < offsets[1])
		return d->segAddrs[1] + addr - offsets[0];
	return d->segAddrs[2] + addr - offsets[1];
}

static u32 AlignPage(u32 in)
{
	return ((in + 0xFFF) / 0x1000) * 0x1000;
}

int KCodeSet::Load3DSXFile(FILE* f, u32 baseAddr)
{
	u32 i, j, k, m;

	_3DSX_Header hdr;
	if (fread(&hdr, sizeof(hdr), 1, f) != 1) {
		XDSERROR("error reading 3DSX header");
		return 2;
	}

	// Endian swap!
#define ESWAP(_field, _type) \
    hdr._field = le_##_type(hdr._field)
	ESWAP(magic, word);
	ESWAP(headerSize, hword);
	ESWAP(relocHdrSize, hword);
	ESWAP(formatVer, word);
	ESWAP(flags, word);
	ESWAP(codeSegSize, word);
	ESWAP(rodataSegSize, word);
	ESWAP(dataSegSize, word);
	ESWAP(bssSize, word);
#undef ESWAP

	if (hdr.magic != _3DSX_MAGIC) {
		XDSERROR("error not a 3DSX file");
		return 3;
	}

	struct _3DSX_LoadInfo d;
	d.segSizes[0] = (hdr.codeSegSize + 0xFFF) &~0xFFF;
	d.segSizes[1] = (hdr.rodataSegSize + 0xFFF) &~0xFFF;
	d.segSizes[2] = (hdr.dataSegSize + 0xFFF) &~0xFFF;
	u32 offsets[2] = { d.segSizes[0], d.segSizes[0] + d.segSizes[1] };
	u32 dataLoadSize = (hdr.dataSegSize - hdr.bssSize + 0xFFF) &~0xFFF;
	u32 bssLoadSize = d.segSizes[2] - dataLoadSize;
	u32 nRelocTables = hdr.relocHdrSize / 4;
	u8* allMem = (u8*)malloc(d.segSizes[0] + d.segSizes[1] + d.segSizes[2] + (4 * 3 * nRelocTables));
	if (!allMem)
		return 3;
	d.segAddrs[0] = baseAddr;
	d.segAddrs[1] = d.segAddrs[0] + d.segSizes[0];
	d.segAddrs[2] = d.segAddrs[1] + d.segSizes[1];
	d.segPtrs[0] = (char*)allMem;
	d.segPtrs[1] = (char*)d.segPtrs[0] + d.segSizes[0];
	d.segPtrs[2] = (char*)d.segPtrs[1] + d.segSizes[1];

	// Skip header for future compatibility.
	fseek(f, hdr.headerSize, SEEK_SET);

	// Read the relocation headers
	u32* relocs = (u32*)((char*)d.segPtrs[2] + hdr.dataSegSize);

	for (i = 0; i < 3; i++)
		if (fread(&relocs[i*nRelocTables], nRelocTables * 4, 1, f) != 1) {
			XDSERROR("error reading reloc header");
			return 4;
		}

	// Read the segments
	if (fread(d.segPtrs[0], hdr.codeSegSize, 1, f) != 1) {
		XDSERROR("error reading code");
		return 5;
	}
	if (fread(d.segPtrs[1], hdr.rodataSegSize, 1, f) != 1) {
		XDSERROR("error reading rodata");
		return 5;
	}
	if (fread(d.segPtrs[2], hdr.dataSegSize - hdr.bssSize, 1, f) != 1) {
		XDSERROR("error reading data");
		return 5;
	}

	// BSS clear
	memset((char*)d.segPtrs[2] + hdr.dataSegSize - hdr.bssSize, 0, hdr.bssSize);

	// Relocate the segments
	for (i = 0; i < 3; i++) {
		for (j = 0; j < nRelocTables; j++) {
			u32 nRelocs = le_word(relocs[i*nRelocTables + j]);
			if (j >= 2) {
				// We are not using this table - ignore it
				fseek(f, nRelocs*sizeof(_3DSX_Reloc), SEEK_CUR);
				continue;
			}

#define RELOCBUFSIZE 512*4
			_3DSX_Reloc relocTbl[RELOCBUFSIZE];

			u32* pos = (u32*)d.segPtrs[i];
			u32* endPos = pos + (d.segSizes[i] / 4);

			while (nRelocs) {
				u32 toDo = nRelocs > RELOCBUFSIZE ? RELOCBUFSIZE : nRelocs;
				nRelocs -= toDo;

				if (fread(relocTbl, toDo*sizeof(_3DSX_Reloc), 1, f) != 1)
					return 6;

				for (k = 0; k < toDo && pos < endPos; k++) {
					//DEBUG("(t=%d,skip=%u,patch=%u)\n", j, (u32)relocTbl[k].skip, (u32)relocTbl[k].patch);
					pos += le_hword(relocTbl[k].skip);
					u32 num_patches = le_hword(relocTbl[k].patch);
					for (m = 0; m < num_patches && pos < endPos; m++) {
						u32 inAddr = (char*)pos - (char*)allMem;
						u32 addr = TranslateAddr(le_word(*pos), &d, offsets);
						//DEBUG("Patching %08X <-- rel(%08X,%d) (%08X)\n", baseAddr + inAddr, addr, j, le_word(*pos));
						switch (j) {
						case 0:
							*pos = le_word(addr);
							break;
						case 1:
							*pos = le_word(addr - inAddr);
							break;
						}
						pos++;
					}
				}
			}
		}
	}

	// Write the data
	m_code = (u8*)calloc(d.segSizes[0], 1);
	m_rodata = (u8*)calloc(d.segSizes[1], 1);
	m_data = (u8*)calloc((dataLoadSize + bssLoadSize), 1);

	memcpy(m_code, allMem, d.segSizes[0]);
	memcpy(m_rodata, allMem + d.segSizes[0], d.segSizes[1]);
	memcpy(m_data, allMem + d.segSizes[0] + d.segSizes[1], dataLoadSize);

	m_code_pages = d.segSizes[0]/0x1000;
	m_rodata_pages = d.segSizes[1] / 0x1000;
	m_data_pages = dataLoadSize / 0x1000;
	m_bss_pages = bssLoadSize / 0x1000;

	free(allMem);

	LOG("CODE:   %u pages", d.segSizes[0] / 0x1000);
	LOG("RODATA: %u pages", d.segSizes[1] / 0x1000);
	LOG("DATA:   %u pages", dataLoadSize / 0x1000);
	LOG("BSS:    %u pages", bssLoadSize / 0x1000);

	return 0; // Success.
}
void KCodeSet::LoadElfFile(u8 *addr)
{
	u32 *header = (u32*)addr;
	u32 *phdr = (u32*)(addr + Read32((u8*)&header[7]));
	u32 n = Read32((u8*)&header[11]) & 0xFFFF;
	u32 i;

	for (i = 0; i < n; i++, phdr += 8) {
		if (phdr[0] != 1) // PT_LOAD
			continue;

		u32 off = Read32((u8*)&phdr[1]);
		u32 dest = Read32((u8*)&phdr[3]);
		u32 filesz = Read32((u8*)&phdr[4]);
		u32 memsz = Read32((u8*)&phdr[5]);

		//round up (this fixes bad malloc implementation in some homebrew)
		memsz = (memsz + 0xFFF)&~0xFFF;

		u8* data = (u8*)calloc(memsz, 1);
		memcpy(data, addr + off, filesz);
		//mem_AddSegment(dest, memsz, data);
		switch (phdr[6] & 0x7)
		{
		case 5: //R_X
			m_code = data;
			m_code_pages = memsz / 0x1000;
			break;
		case 1: //R__
			m_rodata = data;
			m_rodata_pages = memsz / 0x1000;
			break;
		case 3: //RW_
			m_data = data;
			m_data_pages = filesz+0xFFF / 0x1000;
			m_bss_pages = memsz / 0x1000;
			break;
		default:
			XDSERROR("unknown elf section");
			free(data);
			break;
		}
	}
}

bool KCodeSet::Patch()
{
	char temp[0x80];
	sprintf(temp, "HLE\\%s.3dsx", m_name);

	FILE* f = fopen(temp,"rb");
	if (f)
	{
		Load3DSXFile(f, 0x00100000);
		fclose(f);
		return true;
	}
	sprintf(temp, "HLE\\%s.elf", m_name);
	f = fopen(temp, "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		u32 sz = ftell(f);
		fseek(f, 0, SEEK_SET);
		u8* data = new u8[sz];
		fread(data, sz, 1, f);
		LoadElfFile(data);
		delete data;
		fclose(f);
		return true;
	}
	return false;
}
KCodeSet::KCodeSet(u8* code_buf, u32 code_pages, u8* rodata_buf, u32 rodata_pages,
    u8* data_buf, u32 data_pages, u32 bss_pages, u64 TitleID, const char* name)
{
    m_name = strdup(name);
    m_TitleID = TitleID;

	if (Patch())
		return;

    m_code   = (u8*) calloc(code_pages, PAGE_SIZE);
    m_rodata = (u8*) calloc(rodata_pages, PAGE_SIZE);
    m_data   = (u8*) calloc((data_pages + bss_pages), PAGE_SIZE);

    if(m_code == NULL || m_rodata == NULL || m_data == NULL || m_name == NULL) {
        XDSERROR("Out of memory.\n");
        return;
    }

    memcpy(m_code,   code_buf,   code_pages   * PAGE_SIZE);
    memcpy(m_rodata, rodata_buf, rodata_pages * PAGE_SIZE);
    memcpy(m_data,   data_buf,   data_pages   * PAGE_SIZE);

	/*FILE * pFile;
	pFile = fopen("code.code", "wb");
	if (pFile != NULL)
	{
		fwrite(code_buf, 1, code_pages   * PAGE_SIZE, pFile);
		fwrite(rodata_buf, 1, rodata_pages   * PAGE_SIZE, pFile);
		fwrite(data_buf, 1, data_pages   * PAGE_SIZE, pFile);
		fclose(pFile);
	}*/


    memset(m_data + data_pages * PAGE_SIZE, 0, bss_pages * PAGE_SIZE);

    m_code_pages   = code_pages;
    m_rodata_pages = rodata_pages;
    m_data_pages   = data_pages;
    m_bss_pages    = bss_pages;

}

Result KCodeSet::MapInto(KMemoryMap* map,bool spezialmem)
{
    auto chunk_code   = new MemChunk();
    auto chunk_rodata = new MemChunk();
    auto chunk_data   = new MemChunk();

    chunk_code->data   = m_code;
    chunk_rodata->data = m_rodata;
    chunk_data->data   = m_data;

    chunk_code->size   = m_code_pages   * PAGE_SIZE;
    chunk_rodata->size = m_rodata_pages * PAGE_SIZE;
    chunk_data->size   = (m_data_pages + m_bss_pages) * PAGE_SIZE;

    u32 startaddr = spezialmem ? 0x14000000 : 0x00100000;

    if (map->AddPages(startaddr,
        chunk_code->size, chunk_code->data, chunk_code,
        PERMISSION_RX, MEMTYPE_CODE,NULL) != Success)
        return -1;

    if (map->AddPages(startaddr + chunk_code->size,
        chunk_rodata->size, chunk_rodata->data, chunk_rodata,
        PERMISSION_R, MEMTYPE_CODE, NULL) != Success)
        return -1;

    if (map->AddPages(startaddr + chunk_code->size + chunk_rodata->size,
        chunk_data->size, chunk_data->data, chunk_data,
        PERMISSION_RW, MEMTYPE_CODE, NULL) != Success)
        return -1;

    return Success;
}

KCodeSet::~KCodeSet()
{
    free((void*) m_code);
    free((void*) m_rodata);
    free((void*) m_data);
    free((void*) m_name);
}
const char* KCodeSet::GetName()
{
    return m_name;
}
bool KCodeSet::IsInstanceOf(ClassName name) {
    if (name == KCodeSet::name)
        return true;

    return super::IsInstanceOf(name);
}
