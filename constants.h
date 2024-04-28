
enum PACKAGE_TYPE {DROP = 1, CONTAINER = 2, KG_BAG = 3};

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


struct node{
    AidDrop drop;
    struct node* next;
};

typedef struct node ListNode;
typedef ListNode* List;
