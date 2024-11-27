#include "PersistDevList.h"

const char* filename = "persist.bin";

PersistDevList::PersistDevList(int structSize) //: _structSize(structSize)
{
    _structSize = structSize;
    FILE* f_ptr;
    f_ptr = fopen(filename, "rb");
    while (!feof(f_ptr))
    {
        // PersistMQTT newDev;
        PersistGeneric* newDev = (PersistGeneric*)malloc(_structSize);
        fread(&newDev, _structSize, 1, f_ptr);
        _map[newDev->index] = newDev;
    }
    fclose(f_ptr);
}

void PersistDevList::Upsert(int index, void* newDev)
{
    PersistGeneric* persistedDev = (PersistGeneric*)malloc(sizeof(index) + _structSize);
    persistedDev->index = index;
    memcpy(persistedDev->data, newDev, _structSize);
    _map[index] = persistedDev;
    PersistDevList::Persist();
}

void PersistDevList::Persist(void)
{
    FILE* f_ptr;
    f_ptr = fopen(filename, "wb");
    for (auto dev : _map)
    {
        fwrite(dev.second, _structSize, 1, f_ptr);
    }
    fclose(f_ptr);
}

void PersistDevList::Apply(PersistApplyFn func)
{
    for (auto dev : _map)
    {
        func(dev.first, dev.second);
    }
}
