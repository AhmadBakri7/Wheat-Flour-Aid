
enum PACKAGE_TYPE {DROP = 1, CONTAINER = 2, KG_BAG = 3, SORTER_VALUE = 100};

enum {SORTER, DISTRIBUTOR};

typedef struct {
    long package_type; /* to whom this package is being sent (type) */
    int weight;        /* in KG */
} AidPackage;


typedef struct {
    long package_type;  /* to whom this package is being sent (type) */
    int package_number; /**/
    int weight;         /* in KG */
    int amplitude;      /* height from surface */
} AidDrop;

typedef struct{
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


union semun {
    int              val;
    struct semid_ds *buf;
    ushort          *array;
};


