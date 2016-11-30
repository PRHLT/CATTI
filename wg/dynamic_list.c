/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library that implements a linked list */
#include <stdio.h>
#include <stdlib.h>

#include "auxiliar.h"
#include "dynamic_list.h"

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

 /** \d Create dynamic linked list */
list_t *create_list(int number_of_lists){

  list_t *L;
  int i;

  mt(L = (list_t *) malloc(sizeof(list_t) * number_of_lists));

  for(i=0;i<number_of_lists;i++)  
    L[i].head = L[i].tail = NULL;

  return L;

  }


/*****************************************************************************/

/** \d Insert one item in the tail the dynamic linked list */
void insert_into_list(list_t *L, void *data){

  list_element_t *list_item;

  CREATE_LIST_ELEMENT(list_item);
  list_item->data = data;

  /* There is no element in the list, head and tail point to the same element */
  if (L->head == NULL){
    L->head = L->tail = list_item;    
  }
  /* Insert one element in the tail of the list */
  else{
    L->tail->next = list_item;
    L->tail = list_item;    
  }

}

/*****************************************************************************/

/** \d Insert one item in the tail the dynamic linked list */
void insert_into_head_list(list_t *L, void *data){

  list_element_t *list_item;

  CREATE_LIST_ELEMENT(list_item);
  list_item->data = data;

  /* There is no element in the list, head and tail point to the same element */
  if (L->head == NULL){
    L->head = L->tail = list_item;
  }
  /* Insert one element in the head of the list */
  else{
    list_item->next = L->head;
    L->head = list_item;
  }

}
/*****************************************************************************/


/** \d Extract one item from the dynamic linked list */
void *extract_from_list(list_t *L){

  list_element_t *list_item;
  void *data;

  /* If empty list return NULL*/
  if (L->head == NULL)
    return NULL;

  /* Take element in the head */
  list_item = L->head;
  /* and its data */
  data = list_item->data;
  
  /* If we have only one element in the list */
  if (L->head->next == NULL){
    L->head = L->tail = NULL;    
  }
  else{
    L->head = L->head->next;    
  }

  /* Once we have used this list_item, free memory */
  DESTROY_LIST_ELEMENT(list_item);
  
  return data;

}
/*****************************************************************************/

/** \d Read one item from the dynamic linked list and move head to the next element in the list */
/* THIS OPERATION IS ALTERING THE LIST */
void *read_from_list(list_t *L){

  list_element_t *list_item;
  void *data;

  /* If empty list return NULL*/
  if (L->head == NULL)
    return NULL;

  /* Take element in the head */
  list_item = L->head;
  /* and its data */
  data = list_item->data;

  /* If we have only one element in the list */
  if (L->head->next == NULL){
    L->head = L->tail = NULL;
  }
  else{
    L->head = L->head->next;
  }

  return data;

}


/*****************************************************************************/


/** \d Checking if list_item in list */
int in_list(list_t *L, void *data){

  list_element_t *list_item;

  /* We use an auxiliar list to go over the list L*/
  list_item = L->head;
  
  /* If empty list return NULL*/
  while (list_item != NULL){
    if(list_item->data == data)
      return 1;
    else{
      list_item = list_item->next;
    }
  }      

  return 0;
  
}
/*****************************************************************************/

/** \d Removing the first occurrence of element data from the list L */
void remove_from_list(list_t *L, void *data){

  list_element_t *list_item;
  list_element_t *prev_list_item;

  /* We use an auxiliar list to go over the list L*/
  prev_list_item = NULL;
  list_item = L->head;

  /* while not empty list */
  while (list_item != NULL){
    if(list_item->data == data){
      /* If there is only one element in the list */
      if((list_item==L->head)&&(list_item==L->tail)){
        L->head = L->tail = NULL;
        DESTROY_LIST_ELEMENT(list_item);
        break;
      }
      /* If we have at least two elements in the list */
      /* If this element is at the head of the list */
      if(list_item==L->head){
        L->head = list_item->next;
        DESTROY_LIST_ELEMENT(list_item);
        break;
      }
      /* If this element is at the tail of the list */
      if(list_item==L->tail){
        L->tail = prev_list_item;
        L->tail->next = NULL;
        DESTROY_LIST_ELEMENT(list_item);
        break;
      }    
      if((list_item!=L->head)&&(list_item!=L->tail)){
        prev_list_item->next = list_item->next;
        DESTROY_LIST_ELEMENT(list_item);
        break;
      }
    }
    else{
      prev_list_item = list_item;
      list_item = list_item->next;
    }
  }
}
