/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library that implements a linked list */
#ifndef DYNAMIC_LIST_INCLUDED
#define DYNAMIC_LIST_INCLUDED

/******************************************************************************/
/*****************************MACROS*******************************************/
/******************************************************************************/

/* Macros that replace simple functions*/
#define DESTROY_LIST(L) free(L)
#define CREATE_LIST_ELEMENT(list_item)  mt(list_item = (list_element_t *) malloc(sizeof(list_element_t))); list_item->next = NULL
#define DESTROY_LIST_ELEMENT(list_item) free(list_item)
#define NEXT_ELEMENT_IN_LIST(list_item) list_item->next
#define EXTRACT_DATA_FROM_LIST_ELEMENT(list_item) list_item->data
#define EMPTY_LIST(L) (int) !L->head
#define OBTAIN_LIST_TAIL(L) L->tail->data
#define OBTAIN_LIST_HEAD(L) L->head->data
#define COPY_LIST(source_L,target_L) target_L->head = source_L->head; target_L->tail = source_L->tail
#define INITIALIZE_LIST(L) L->head = L->tail = NULL
#define LIST_HEAD(L) L->head
#define LIST_TAIL(L) L->tail

/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

/** \d List element */
typedef struct list_element{
  void *data;
  struct list_element *next;
}list_element_t;

/** \d Data structure for list */
typedef struct list{
  list_element_t *head; /**< \d Pointer to first element in the list  */
  list_element_t *tail; /**< \d Pointer to last element in the list */
}list_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Create dynamic linked list */
list_t *create_list(int number_of_lists);

/* Destroy dynamic linked list */
/* REPLACED BY A MACRO */
/*
void destroy_list(list_t *L);
*/

/* Create list element */
/* REPLACED BY A MACRO */
/*
list_element_t *create_list_element(void);
*/

/* Destroy list element */
/* REPLACED BY A MACRO */
/*
void destroy_list_element(list_element_t *item);
*/

/** \d Insert one item in the tail of the dynamic linked list */
void insert_into_list(list_t *L, void *data);

/** \d Insert one item in the tail the dynamic linked list */
void insert_into_head_list(list_t *L, void *data);

/** \d Extract one item from the dynamic linked list */
void *extract_from_list(list_t *L);

/** \d Read one item from the dynamic linked list and move head to the next element in the list */
/* THIS OPERATION IS ALTERING THE LIST */
void *read_from_list(list_t *L);

/* Checking if list is empty */
/* REPLACED BY A MACRO */
/*
int empty_list(list_t *L);
*/
/** \d Checking if list_item in list */
int in_list(list_t *L, void *data);

/** \d Removing a given element data from the list L */
void remove_from_list(list_t *L, void *data);

/* Obtain the tail of list without altering the list */
/* REPLACED BY A MACRO */
/*
void *obtain_list_tail(list_t *L);
*/

/* Obtain the head of list without altering the list */
/* REPLACED BY A MACRO */
/*
void *obtain_list_head(list_t *L);
*/

/* Copy a source list source_L into a target list target_L */
/* REPLACED BY A MACRO */
/*
void copy_list(list_t *source_L, list_t *target_L);
*/

#endif
