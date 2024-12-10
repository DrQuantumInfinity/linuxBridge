#include "PersistDevList.h"
#include "Log.h"

PersistDevList::PersistDevList(size_t userDataSize, const char* filename)
{
    _structSize = userDataSize + sizeof(PersistGeneric);
    strncpy(_filename, filename, sizeof(_filename));
    FILE* f_ptr;
    f_ptr = fopen(filename, "rb");
    if(f_ptr!=NULL)
    {
        while (!feof(f_ptr))
        {
            PersistGeneric* pNewDev = (PersistGeneric*)malloc(_structSize);
            if(1 == fread(pNewDev, _structSize, 1, f_ptr))
            {
                log_info("Fread in persist. index is:%d", pNewDev->index);
                _map[pNewDev->index] = pNewDev;
            }
            else
            {
                log_error("Fread of persist file failed. filename: %s", _filename);
            }
        }
        fclose(f_ptr);
    }
}

void PersistDevList::Upsert(int index, void* newDev)
{
    PersistGeneric* persistedDev = (PersistGeneric*)malloc(_structSize);
    persistedDev->index = index;
    memcpy(persistedDev->data, newDev, _structSize - sizeof(PersistGeneric));
    _map[index] = persistedDev;
    PersistDevList::Persist();
}

void PersistDevList::Persist(void)
{
    FILE* f_ptr;
    f_ptr = fopen(_filename, "wb");
    for (auto dev : _map)
    {
        fwrite(dev.second, _structSize , 1, f_ptr);
    }
    fclose(f_ptr);
}

void PersistDevList::Apply(PersistApplyFn func)
{
    log_info("Applying persist. map size: %d,  filename: %s", _map.size(), _filename);
    for (auto dev : _map)
    {
        func(dev.first, dev.second->data);
    }
}
