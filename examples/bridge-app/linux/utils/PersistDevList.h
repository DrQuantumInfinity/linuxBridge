#include "EndpointApi.h"
#include "transportMqtt.h"

struct PersistMQTT {
    int index;
    char pName[MQTT_MAX_DEVICE_NAME_LENGTH];
    char room[ENDPOINT_LOCATION_LENGTH];
    MQTT_TYPE type;
};

typedef std::map<int, PersistMQTT> MAPPING;

class PersistDevList {
public:
    PersistDevList(void);
    void Upsert(PersistMQTT newDev);

private:
    MAPPING _map;
    void Persist(void);
};