#pragma once

#include <map>

struct PersistGeneric {
    int index;
    uint8_t data[];
};
typedef std::map<int, PersistGeneric*> PersistentMap;
typedef void (*PersistApplyFn)(int /*index*/, void* /*newDev*/);
class PersistDevList {
public:
    PersistDevList(size_t structSize, const char* filename);
    void Remove(int index);
    void Upsert(int index, void* newDev);
    void Apply(PersistApplyFn func);
private:
    void Persist(void);
    size_t _structSize;
    PersistentMap _map;
    char _filename[64];
};