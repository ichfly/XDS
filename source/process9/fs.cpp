#include "Kernel.h"
#include "Hardware.h"
#include "Process9.h"
#include "process9/archive.h"

#define LOGFS

P9FS::P9FS(Process9* owner) : m_owner(owner)
{
}
P9FS::~P9FS()
{

}
void P9FS::Command(u32 data[], u32 numb)
{
    u32 resdata[0x200];
    memset(resdata, 0, sizeof(resdata));
    u16 cmd = (data[0] >> 16);
	resdata[0] = 0x00000040;
	resdata[1] = 0x00000000;
    switch (cmd)
    {
	case 0x01: //OpenFile
	{
		u32 transaction = data[1];
		u64 handle = (data[2] >> 0) | ((u64)(data[3]) << 32);
		u32 file_lowpath_type = data[4];
		u32 file_lowpath_sz = data[5];
		u32 flags = data[6];
		u32 attr = data[7];
		u32 file_lowpath_desc = data[8];
		u32 file_lowpath_ptr = data[9];

#ifdef LOGFS
		char tmp[256];
		LOG("FS OpenFile");
		LOG("   archive_handle=%"PRIx64, handle);
		LOG("   flags = %s", P9File::FlagsToString(flags, tmp));
#endif

		auto a = m_open.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}

		u64 p9file_handle = 0;
		P9File* P9file = 0;
		u32 result = 0xc8804464; //TODO: Find proper error code

		if (a)
		{
			//This is freed in the destructor of LowPath
			u8 *lowpath_data = new u8[file_lowpath_sz];
			for (u32 i = 0; i < file_lowpath_sz; i++)
			{
				u8 sdata = 0;
				s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(file_lowpath_desc >> 4) & 0xF]->Read8(i + file_lowpath_ptr, sdata);
				lowpath_data[i] = sdata;
			}

			LowPath lowpath(file_lowpath_type, file_lowpath_sz, file_lowpath_desc, lowpath_data);
			P9file = a->data->Archobj->OpenFile(&lowpath, flags, attr, &result);
			/*delete lowpath_data;
			delete lowpath;*/ //todo fix me plz
		}
		if (P9file)
		{
			s_fsFileentry *a = new s_fsFileentry;
			a->id = lastID++;
			p9file_handle = a->id;
			a->Archobj = P9file;
			m_fopen.AddItem(a);
			result = 0;
		}
		else
		{
			LOG("error opening file");
		}

#ifdef LOGFS
		LOG("   p9file_handle=%"PRIx64, p9file_handle);
#endif

		resdata[0] = 0x000100C1;
		resdata[1] = result;
		resdata[2] = (u32)p9file_handle;
		resdata[3] = (u32)(p9file_handle >> 32);
		resdata[4] = 0x4; //this is needed
		break;
	}
	case 0x02: //DeleteFile
	{
		u32 transaction = data[1];
		u64 handle = (data[2] >> 0) | ((u64)(data[3]) << 32);
		u32 file_lowpath_type = data[4];
		u32 file_lowpath_sz = data[5];
		u32 file_lowpath_desc = data[6];
		u32 file_lowpath_ptr = data[7];

#ifdef LOGFS
		LOG("FS DeleteFile");
		LOG("   archive_handle=%"PRIx64, handle);
#endif

		resdata[0] = 0x00020142;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error

		auto a = m_open.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}

		if (a)
		{
			//This is freed in the destructor of LowPath
			u8 *lowpath_data = new u8[file_lowpath_sz];
			for (u32 i = 0; i < file_lowpath_sz; i++)
			{
				u8 sdata = 0;
				s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(file_lowpath_desc >> 4) & 0xF]->Read8(i + file_lowpath_ptr, sdata);
				lowpath_data[i] = sdata;
			}

			auto lowpath = new LowPath(file_lowpath_type, file_lowpath_sz, file_lowpath_desc, lowpath_data);

			u32 result = 0;
			a->data->Archobj->DeleteFile(lowpath, &result);
			delete lowpath;

			if (result == -1)
				result = 0xC8804478; //ENOENT

			resdata[0] = 0x00020041;
			resdata[1] = result;
		}
		break;
	}
	case 0x09: //ReadFile
	{
		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
		u64 file_offset = (data[3] >> 0) | ((u64)(data[4]) << 32);
		u32 size = data[5];
		u32 desc_read = data[6];
		u32 ptr_read = data[7];

#ifdef LOGFS
		LOG("FS ReadFile %08x %08x %08x", size, desc_read, ptr_read);
		LOG("   file_handle=%"PRIx64, handle);
		LOG("   file_offset=%"PRIx64, file_offset);
#endif
		resdata[0] = 0x00090142;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error

		u32 buffsize = size;
		u8 *buffer = new u8[buffsize];
		memset(buffer, 0, buffsize);

		auto a = m_fopen.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}
		if (a)
		{
			u32 out_size = 0;
			a->data->Archobj->read(buffer, size, file_offset, out_size);

			for (u32 i = 0; i < out_size; i++)
			{
				s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(desc_read >> 4) & 0xF]->Write8(i + ptr_read, buffer[i]);
			}

			resdata[0] = 0x00090081;
			resdata[1] = 0;
			resdata[2] = out_size;
			resdata[3] = 0x4; //this is needed
		}
		break;
	}
	case 0xA: //CalculateFileHashSHA256
	{
		resdata[0] = 0x000A0041; //0x000A00C2
		resdata[1] = 0xFFFFFFFF; //todo get the correct error
		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
		u32 size_hashtable = data[3];
		u32 desc_hashtable = data[4];
		u32 ptr_hashtable = data[5];
#ifdef LOGFS
		LOG("FS CalculateFileHashSHA256");
		LOG("   file_handle=%"PRIx64, handle);
		LOG("   size=%08X", size_hashtable);
		LOG("   ptr=%08X", ptr_hashtable);
#endif
		auto a = m_fopen.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}
		if (a)
		{
			resdata[0] = 0x000A0041;
			resdata[1] = 0x0;
			resdata[2] = 4;
			u8* hash = a->data->Archobj->GetHashPtr();

			for (u32 i = 0; i < size_hashtable; i++)
			{
				s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(desc_hashtable >> 4) & 0xF]->Write8(i + ptr_hashtable, hash[i]);
			}
		}
		break;
	}
	case 0x0B: //WriteFile
	{
		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
		u64 file_offset = (data[3] >> 0) | ((u64)(data[4]) << 32);
		u32 size = data[5];
		u32 unk = data[6];
		u32 desc_write = data[7];
		u32 ptr_write = data[8];

#ifdef LOGFS
		LOG("FS WriteFile %08x %08x %08x", size, desc_write, ptr_write);
		LOG("   file_handle=%08X", handle);
		LOG("   file_offset=%"PRIx64, file_offset);
#endif
		resdata[0] = 0x000B0182;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error

		u32 buffsize = size;
		u8 *buffer = new u8[buffsize];
		memset(buffer, 0, buffsize);

		auto a = m_fopen.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}
		if (a)
		{
			for (u32 i = 0; i < size; i++)
			{
				s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(desc_write >> 4) & 0xF]->Read8(i + ptr_write, buffer[i]);
			}

			u32 out_size = 0;
			a->data->Archobj->write(buffer, size, file_offset, out_size);

			resdata[0] = 0x000B0081;
			resdata[1] = 0;
			resdata[2] = out_size;
			resdata[3] = 0x4; //this is needed
		}
		break;
	}
	case 0xC: //CalcSavegameMAC
	{
		resdata[0] = 0x000C0104;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error

		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
		u32 out_size = data[3];
		u32 in_size = data[4];
		u32 in_desc = data[5];
		u32 in_ptr = data[6];
		u32 out_desc = data[7];
		u32 out_ptr = data[8];
#ifdef LOGFS
		LOG("FS CalcSavegameMAC");
		LOG("   file_handle=%"PRIx64, handle);
		LOG("   in  size=%08X, ptr=%08X", in_size, in_ptr);
		LOG("   out size=%08X, ptr=%08X", out_size, out_ptr);
#endif
		for (u32 i = 0; i < in_size; i++)
		{
			u8 data;
			m_owner->m_kernel->m_IPCFIFOAdresses[(in_desc >> 4) & 0xF]->Read8(in_ptr + i, data);
			printf("%02x ", data);
		}
		LOG("");
		for (u32 i = 0; i < out_size; i++)
		{
			//s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(out_desc >> 4) & 0xF]->Write8(i + out_ptr, rand() % 255);
			s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(out_desc >> 4) & 0xF]->Write8(i + out_ptr, 0x11);
		}

		resdata[0] = 0x000C0041;
		resdata[1] = 0;
		break;
	}
	case 0xD: //GetFileSize
	{
		resdata[0] = 0x000D0040;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error
		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
#ifdef LOGFS
		LOG("FS GetFileSize=%"PRIx64, handle);
#endif
		auto a = m_fopen.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}
		if (a)
		{
			resdata[0] = 0x000D00C0;
			resdata[1] = 0x0;
			u64 size = a->data->Archobj->getsize();
			resdata[2] = (u32)size;
			resdata[3] = size>>32;
		}
		break;
	}
	case 0xE: //SetFileSize
	{
		resdata[0] = 0x000E0100; //0x000A00C2
		resdata[1] = 0xFFFFFFFF; //todo get the correct error
		u64 size = (data[1] >> 0) | ((u64)(data[2]) << 32);
		u64 handle = (data[3] >> 0) | ((u64)(data[4]) << 32);
#ifdef LOGFS
		LOG("FS SetFileSize");
		LOG("   file_handle=%"PRIx64, handle);
		LOG("   size=%"PRIx64, size);
#endif
		auto a = m_fopen.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}
		if (a)
		{
			resdata[0] = 0x000E0040;
			resdata[1] = a->data->Archobj->setsize(size);
		}
		break;
	}
	case 0xF: //CloseFile 
	{
		resdata[0] = 0x000F0040;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error
		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
		LOG("CloseFile=%"PRIx64, handle);
		auto a = m_fopen.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}
		if (a)
		{
			delete a->data->Archobj;
			m_fopen.RemoveItem(a);
			resdata[1] = 0;
		}
		break;
	}
    case 0x12: //OpenArchive
    {
        resdata[0] = 0x00120040;
        resdata[1] = 0xFFFFFFFF; //todo get the correct error

        u32 file_lowpath_type = data[2];
        u32 file_lowpath_sz = data[3];
        u32 file_lowpath_desc = data[4];
        u32 file_lowpath_ptr = data[5];

        //This is freed in the destructor of LowPath
        u8 *lowpath_data = new u8[file_lowpath_sz];
        for(u32 i = 0; i < file_lowpath_sz; i++)
        {
            u8 sdata = 0;
            s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(file_lowpath_desc >> 4) & 0xF]->Read8(i + file_lowpath_ptr, sdata);
            lowpath_data[i] = sdata;
        }

        auto lowpath = new LowPath(file_lowpath_type, file_lowpath_sz, file_lowpath_desc, lowpath_data);

        s_fsArchiveEntry *a = new s_fsArchiveEntry;
        m_open.AddItem(a);
        a->id = lastID++;

#ifdef LOGFS
        LOG("FS OpenArchive stub %08x %08x %08x %08x", data[1], data[2], data[3], data[4]);
        LOG("   archive_handle=%"PRIx64, a->id);
#endif
		a->Archobj = NULL;
		resdata[1] = 0;
		try
		{
			switch (data[1])
			{
			case 0x1234567b: // ExtSaveData, and ExtSaveData for BOSS
				a->Archobj = new Archive1234567b(this->m_owner, lowpath);
				break;
			case 0x1234567c: // SystemSaveData
				a->Archobj = new Archive1234567c(this->m_owner, lowpath);
				break;
			case 0x1234567d: // NAND RW 
				a->Archobj = new Archive1234567d(this->m_owner, lowpath);
				break;
			case 0x1234567e: // NAND RO
				a->Archobj = new Archive1234567e(this->m_owner, lowpath);
				break;
			case 0x2345678a: //User/GameCard SaveData (for check), and other uses (FS can only mount the latter) (lo hi mediatype reserved) 
				a->Archobj = new Archive2345678a(this->m_owner, lowpath);
				break;
			case 0x2345678e: // SaveData, ExeFS, and RomFS (For fs:LDR, only ExeFS)
				a->Archobj = new Archive2345678e(this->m_owner, lowpath);
				break;
			case 0x567890B0: // NAND CTR FS
				a->Archobj = new Archive567890b0(this->m_owner, lowpath);
				break;
			default:
				throw 0xc8804464;
				break;
			}
		}
		catch (u32 val)
		{
			resdata[1] = val;
		}
		resdata[0] = 0x001200c1;
        resdata[2] = (u32)a->id;
        resdata[3] = (u32)(a->id >> 32);
        resdata[4] = 0x4; //this is needed
        delete lowpath;
        break;
    }
	case 0x13:
	{
		LOG("FS update Quota.dat stub Archive: %08x%08x unk: %08x size: %08x", data[1], data[2], data[3], data[4]);

		char* str = new char[data[4] + 1];
		memset(str, 0, data[4] + 1);
		for (u32 i = 0; i < data[4]; i++) //Quota.dat
		{
			u8 sdata = 0;
			m_owner->m_kernel->m_IPCFIFOAdresses[(data[5] >> 4) & 0xF]->Read8(i + data[6], sdata);
			str[i] = sdata;
		}
		LOG("%s",str);
		resdata[0] = 0x00130040;
		resdata[1] = 0x00000000;
		resdata[2] = 0x00000000; //u8
		delete str;
		break;
	}
	case 0x16: //CloseArchive
	{
		resdata[0] = 0x00160080;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error

		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);

#ifdef LOGFS
		LOG("FS CloseArchive - stubbed");
		LOG("   archive_handle=%"PRIx64, handle);
#endif

		auto a = m_open.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}

		if (a)
		{
			//TODO: Should we really delete this or just ignore this call as kernel calls ReopenArchive with same handleid soon after closing it
			m_open.RemoveItem(a);
			delete a->data->Archobj;
			delete a->data;
		}

		resdata[0] = 0x00160040;
		resdata[1] = 0;
		break;
	}
	case 0x17: //Unk17
	{
		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
#ifdef LOGFS
		LOG("FS Unk17 - stubbed");
		LOG("   archive_handle=%"PRIx64, handle);
#endif
		resdata[0] = 0x00170080;
		resdata[1] = 0;
		resdata[2] = 1; //this is a unsigned byte
		break;
	}
	case 0x18: //GetCardType
	{
#ifdef LOGFS
		LOG("GetCardType - stubbed");
#endif
		resdata[0] = 0x00180080;
		resdata[1] = 0;
		resdata[2] = 0; //this is a unsigned byte
		break;
	}
	case 0x1C: //GetSdmcDetected
	{
#ifdef LOGFS
		LOG("GetSdmcDetected - stubbed");
#endif
		resdata[0] = 0x001C0080;
		resdata[1] = 0;
		resdata[2] = 0; //no
		break;
	}
	case 0x1D: //GetSdmcWritable
	{
#ifdef LOGFS
		LOG("GetSdmcWritable - stubbed");
#endif
		resdata[0] = 0x001D0080;
		resdata[1] = 0;
		resdata[2] = 0; //no
		break;
	}
	case 0x26: //GetCardSlotInserted
	{
#ifdef LOGFS
		LOG("GetCardSlotInserted - stubbed");
#endif
		resdata[0] = 0x00260080;
		resdata[1] = 0;
		resdata[2] = 0; //no
		break;
	}
	case 0x4D: //ReadFileWrapper
	{
		//This probably actually just checks that the hash is what it should be during read if not returns different error code
		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
		u64 file_offset = (data[3] >> 0) | ((u64)(data[4]) << 32);
		u32 size = data[5];
		u32 alinement = data[6];
		u32 size_hashtable = data[7];
		u32 desc_hashtable = data[8];
		u32 ptr_hashtable = data[9];
		u32 desc_read = data[10];
		u32 ptr_read = data[11];

#ifdef LOGFS
		LOG("FS ReadFileWrapper (SHA-256 not implemented) %08x %08x %08x", size, alinement, size_hashtable);
		LOG("   file_handle=%"PRIx64, handle);
		LOG("   file_offset=%"PRIx64, file_offset);
		LOG("   pointer=%08x", ptr_read);
		LOG("   modulename=%s", m_owner->m_kernel->m_IPCFIFOAdresses[(desc_read >> 4) & 0xF]->m_process->GetName());
#endif
		resdata[0] = 0x004D0040;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error

		u32 buffsize = size;
		if (size % alinement)
			buffsize = ((buffsize + alinement) / alinement)*alinement;
		u8 *buffer = new u8[buffsize];
		memset(buffer, 0, buffsize);

		auto a = m_fopen.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}
		if (a)
		{
			u32 out_size = 0;
			a->data->Archobj->read(buffer, size, file_offset, out_size);
			
			for (u32 i = 0; i < out_size; i++)
			{
				s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(desc_read >> 4) & 0xF]->Write8(i + ptr_read, buffer[i]);
			}

			resdata[0] = 0x004D0081;
			resdata[1] = 0;
			resdata[2] = out_size;
			resdata[3] = 0x4; //this is needed
		}
		break;
	}
	case 0x4E: //WriteFileWrapper
	{
		u64 handle = (data[1] >> 0) | ((u64)(data[2]) << 32);
		u64 file_offset = (data[3] >> 0) | ((u64)(data[4]) << 32);
		u32 size = data[5];
		u32 unk = data[6];
		u32 size_hashtable = data[7];
		u32 unk2 = data[8];
		u32 desc_write = data[9];
		u32 ptr_write = data[10];
		u32 desc_hash = data[11];
		u32 ptr_hash = data[12];

#ifdef LOGFS
		LOG("FS WriteFileWrapper %08x %08x %08x %08x %08x", size, desc_write, ptr_write, unk, size_hashtable);
		LOG("   file_handle=%08X", handle);
		LOG("   file_offset=%"PRIx64, file_offset);
#endif
		resdata[0] = 0x000B0182;
		resdata[1] = 0xFFFFFFFF; //todo get the correct error

		u32 buffsize = size;
		u8 *buffer = new u8[buffsize];
		memset(buffer, 0, buffsize);

		auto a = m_fopen.list;
		while (a)
		{
			if (a->data->id == handle)
				break;
			a = a->next;
		}
		if (a)
		{
			for (u32 i = 0; i < size; i++)
			{
				s32 ret = m_owner->m_kernel->m_IPCFIFOAdresses[(desc_write >> 4) & 0xF]->Read8(i + ptr_write, buffer[i]);
			}

			u32 out_size = 0;
			a->data->Archobj->write(buffer, size, file_offset, out_size);

			resdata[0] = 0x000B0081;
			resdata[1] = 0;
			resdata[2] = out_size;
			resdata[3] = 0x4; //this is needed
		}
		break;
	}
    case 0x4F: //0x004F0080 only data[1] used stored in seperate container (Bit(0 - 12) at 080949f0 + 0xC, Bit(12-24) 080949f0 + 0xE) nothing else done here
#ifdef LOGFS
        LOG("FS unk 0x4F stub (uppern 8 Bit unused)%08x (unused) %08x", data[1], data[2]);
#endif
        resdata[0] = 0x004F0040;
        resdata[1] = 0x00000000;
        break;
    case 0x50: //dose nothing
#ifdef LOGFS
        LOG("FS init stub %u", data[1]);
#endif
        resdata[0] = 0x00500040;
        resdata[1] = 0x00000000;
        break;

    default:
        LOG("unknown FS cmd %08x (%08x,%08x,%08x,%08x,%08x,%08x)", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        break;
    }
    m_owner->Sendresponds(numb, resdata);
}
