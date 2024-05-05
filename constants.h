
enum MESSAGE_TYPE {DROP = 1, CONTAINER = 2, KG_BAG = 3, SORTER_VALUE = 100, EMERGENCY = 200};
enum PROCESS_TYPE {PLANE = 1, COLLECTOR, DISTRIBUTOR, FAMILY, SKY};

typedef struct {
    long package_type; /* to whom this package is being sent (type) */
    int weight;        /* in KG */
} AidPackage;


typedef struct {
    long package_type;  /* to whom this package is being sent (type) */
    int weight;         /* in KG */
    int amplitude;      /* height from surface */
} AidDrop;


typedef struct {
    long type;
    int family_index;
    int num_bags_required;
} familyCritical;


typedef struct{
    long familyIndex;
    int starvationRate;
} familyStruct;


typedef struct {
    pid_t plane;
    int amplitude;
} AirSpace;


typedef struct {
    long process_type; /* news type (what kind of process sent this message) */
    int process_index;
} NewsReport;


typedef struct {
    long type;
    char arguments[1000];
} SwapInfo;

union semun {
    int              val;
    struct semid_ds *buf;
    ushort          *array;
};
