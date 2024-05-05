
enum MESSAGE_TYPE {DROP = 1, CONTAINER = 2, KG_BAG = 3, SORTER_VALUE = 100, EMERGENCY = 200};
enum PROCESS_TYPE {PLANE = 1, COLLECTOR, SPLITTER, DISTRIBUTOR, FAMILY, SKY, SORTER, OCCUPATION};

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



/* --------------------------------| openGl objects |-------------------------------- */
typedef struct {
    int plane_number;
    int num_containers;
    int amplitude;
    bool refilling;
    bool destroyed;
} GUI_Plane;

typedef struct {
    int number;
    int amplitude;
    int weight;
} GUI_Drop;

typedef struct {
    int energy;
    int number;
    int containers;
    bool killed;
} GUI_Collector;

typedef struct {
    int energy;
    int number;
    int weight;
    bool swapped;
} GUI_Splitter;

typedef struct {
    int energy;
    int bags;
    int number;
    bool killed;
} GUI_Distributor;

typedef struct {
    int starvation_rate;
    bool alive;
    int number;
} GUI_Family;


typedef struct {
    long type;
    int operation;
    
    union {
        struct {
            int starvation_rate;
            int number;
            bool alive;
        } families;
        
        struct {
            int energy;
            int number;
            int containers;
            bool killed;
        } collector;

        struct {
            int energy;
            int number;
            int weight;
            bool swapped;
        } splitter;

        struct {
            int energy;
            int number;
            int bags;
            bool killed;
        } distributor;
        
        struct {
            int drop_number;
            int weight;
            int amplitude;
        } sky;
        
        struct {
            int num_containers;
            int plane_number;
            int amplitude;
            bool refilling;
            bool destroyed;
        } planes;

    } data;

} MESSAGE;


// typedef struct {
//     long type;
    
//     // families
//     int starvation_rate;
//     bool alive;
    
//     // workers
//     int energy;
//     int bags; /* distributor */

//     // drops
//     int number;
//     int weight;

//     // planes     
//     int num_containers;
//     int plane_number;
//     int amplitude;
//     bool refilling;
// } MESSAGE;

union semun {
    int              val;
    struct semid_ds *buf;
    ushort          *array;
};
