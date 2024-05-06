
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
    int number;
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
    pid_t pid;
} GUI_Collector;

typedef struct {
    int energy;
    int number;
    int weight;
    bool swapped;
    pid_t pid;
} GUI_Splitter;

typedef struct {
    int energy;
    int bags;
    int number;
    bool killed;
    pid_t pid;
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
            pid_t pid;
        } collector;

        struct {
            int energy;
            int number;
            int weight;
            pid_t pid;
        } splitter;

        struct {
            int energy;
            int number;
            int bags;
            bool killed;
            pid_t pid;
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

        struct {
            int worst_fam_index;
            int worst_fam_starve_rate;
            int bags_required;
        } sorter;

    } data;

} MESSAGE;

union semun {
    int              val;
    struct semid_ds *buf;
    ushort          *array;
};
