
enum PACKAGE_TYPE {DROP = 1, CONTAINER, KG_BAG};

typedef struct {
    long package_type; /* to whom this package is being sent (type) */
    int weight;        /* in KG */
} AidPackage;


typedef struct {
    long package_type; /* to whom this package is being sent (type) */
    int weight;        /* in KG */
    int amplitude;     /* height from surface */
} AidDrop;


struct node{
    AidDrop drop;
    struct node* next;
};

typedef struct node ListNode;
typedef ListNode* List;
