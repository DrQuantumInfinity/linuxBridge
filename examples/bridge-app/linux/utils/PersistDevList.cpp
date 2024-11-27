#include "PersistDevList.h"

const char* filename = "persist.bin";

PersistDevList::PersistDevList()
{
    FILE* f_ptr;
    f_ptr = fopen(filename, "rb");
    while (!feof(f_ptr)) {
        PersistMQTT newDev;
        fread(&newDev, sizeof(newDev), 1, f_ptr);
        _map[newDev.index] = newDev;
    }
    fclose(f_ptr);
}

void PersistDevList::Upsert(PersistMQTT newDev)
{
    _map[newDev.index] = newDev;
    PersistDevList::Persist();
}

void PersistDevList::Persist(void)
{
    FILE* f_ptr;
    f_ptr = fopen(filename, "wb");
    for (auto dev : _map)
        fwrite(&dev.second, sizeof(dev.second), 1, f_ptr);
    fclose(f_ptr);
}