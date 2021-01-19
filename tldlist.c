/*
 * Jake Haakanson - 2407682h, SP Exercise 1a
 * This is my own work as defined in the Academic Ethics Agreenment I have signed0
 */

#include <stdlib.h>
#include <string.h>
#include "tldlist.h"


#define GETHEIGHT(n) ((n == NULL) ? -1 : n->height)                                                 //Get height of a subtree rooted at n
#define SETBALANCE(n) ({REHEIGHT(n); n->balance = GETHEIGHT(n->right) - GETHEIGHT(n->left);})       //Sets balance of TLDNode n

#define REHEIGHT(n) ({if (n != NULL) { n->height = 1 + \
    ( GETHEIGHT(n->left) > GETHEIGHT(n->right) ? GETHEIGHT(n->left) : GETHEIGHT(n->right) ); };})   //Re-determine height of TLDNode n


/* --- Aditional function declarations --- */
char *getLabel(char *p, char *c);        	//Recursively retrieves TLD of given hostname string
void rebalance(TLDList *tld, TLDNode *p);	//Re-determines balance of a TLDNode

TLDNode *TLDMin(TLDNode *n);				//Returns minimum node in subtree of n
TLDNode *TLDMax(TLDNode *n);				//Returns maximum node in subtree of n

TLDNode *createNewNode(char *nodeLabel); 	//Creates a new TLDNode with node->name equal to nodeLabel
TLDNode *succ(TLDNode *n);               	//Return the successor of node n, used by the iterator

TLDNode *rotateLeft(TLDNode *n);         	//Rotates n left
TLDNode *rotateRight(TLDNode *n);        	//Rotates n right
TLDNode *rotateLeftThenRight(TLDNode *n);	//Rotates n->left left then n right
TLDNode *rotateRightThenLeft(TLDNode *n);	//Rotates n->right right then n left


/*TLD List (AVL Tree)*/
struct tldlist {
    struct tldnode *root; //Root node of the tree
    Date *begin, *end;    //Start and end dates
    long count;           //Count of nodes in tree
};


/*TLD Node*/
struct tldnode {
	struct tldnode *next;   //Used for converting to linked list during tldlist destruction, ensures address sanitisation
	
    struct tldnode *left, *right, *parent;
    int balance, height;
    long total;
    char *name;
};


/*Iterator for AVLTree iteration using successor function*/
struct tlditerator {
    struct tldnode *node, *max;
};


/*Creates new empty TLDList with begin and end dates*/
TLDList *tldlist_create(Date *begin, Date *end) {
    if (begin != NULL && end != NULL) { //Ensure that date creation was successful
		TLDList *newT;

        if ((newT = (struct tldlist *)malloc(sizeof(struct tldlist))) != NULL) {
            newT->root  = NULL;
            newT->begin = begin;
            newT->end   = end;
            newT->count = 0;
        }

		return newT;

    } else {
        return NULL;	//Return NULL if dates NULL

    }
}


/*Destroys TLDList and all associated Nodes*/
void tldlist_destroy(TLDList *tld) {
     /**********************************************************************************************************************************************************\
     * In order to prevent memory leaks and ensure address sanitisation we must make sure that all pointers to external nodes are updated to NULL.				*
     * One solution to this problem is to convert the AVL Tree to a Singly Linked List of nodes with just one pointer to the next node and all others equal		*
     * to NULL, this way as we iterate with a trailing prev Node, we can set the prev node's pointer to the next node to NULL and free the prev node.			*
     * We use the next pointer instead of an already existing pointer due to the fact that the successor function (succ) must make use of existing pointers.	*
     \**********************************************************************************************************************************************************/

	if (tld != NULL) {		//Ensure tld exists
		TLDIterator *iter;	//Iterator used to iterate over TLDList
		
		TLDNode *prev, *current;
		TLDNode *firstInList = TLDMin(tld->root);	//Minumum used as start of linked list, works due to inorder traversal by iterator
		
		if ((iter = tldlist_iter_create(tld)) != NULL) {	//Ensure iterator creation was successful
			current = tldlist_iter_next(iter);		//Current node in iteration
		
			while (current != NULL) {	//Convert TLDList in AVLTree form into SinglyLinkedList
				prev = current;
				current = tldlist_iter_next(iter);
				prev->next = current;
			}
			
			tldlist_iter_destroy(iter); //Destroy iterator
		
			current = firstInList;		//Get first item in linked list (tree minimum)
			while (current != NULL) {	//Iterate list
				free(current->name);	//un-needed pointers are freed or set to NULL to ensure address sanitisation
				current->name = NULL;
				current->left = current->right = current->parent = NULL;
				current = current->next;
			}
			
			tld->root = NULL;			//Set tld's pointers to NULL before we free any nodes
			tld->begin = tld->end = NULL;
			
			current = firstInList;		//Iterate over linked list starting at the first item
			while (current != NULL) {
				prev = current;			//Trails current
				current = prev->next;	//Update current
				
				prev->next = NULL;		//Set pointer to 'current' to NULL
				free(prev);
			}

			free(tld);					//Free tld list
		}
	}
}


/*Adds new tldnode to tldtree, returns 1 if entry counted, 0 if not*/
int tldlist_add(TLDList *tld, char *hostname, Date *d) {
    if (tld == NULL || d == NULL || date_compare(d, tld->begin) < 0 || date_compare(d, tld->end) > 0) {
        return 0;	//Return 0 if: tld is NULL, date is NULL, or falls outside range
    }
    
    char *nodeLabel = getLabel(hostname, strtok(hostname, "."));    //Get tld of hostname
    
    if (tld->root == NULL) {    //Check if root node is NULL, if it is then the new node is root
        TLDNode *n;
        
        if ((n = createNewNode(nodeLabel)) != NULL) {
            n->total++;
            tld->root = n;
            tld->count++;
            
            return 1;
            
        } else {
            return 0;
            
        }
        
    } else {    //Root node already exists
        TLDNode *current, *trailing;    //Current node and trailing parent
        current = tld->root;
        
        int stringComp; //Used to compare hostname with 'current' node name
        
        while (current != NULL) {   //Locate insertion position using node and trailing parent
            trailing = current;
            stringComp = strcmp(nodeLabel, current->name);
            
            if (stringComp < 0) {   //Use stringComp value to determine if left or right is chosen
                current = current->left;
                
            } else if (stringComp > 0) {
                current = current->right;
                
            } else {    //If stringComp is 0 then the values match, simply increment the current node's total and tld's count
                current->total++;
                tld->count++;
                
                return 1;
            }
        }
        
        TLDNode *n; //Insertion position found so create a new node
        
        if ((n = createNewNode(nodeLabel)) != NULL) {
            n->parent = trailing;   //Assign parent of new node and initialise total
            n->total++;
            
            if (stringComp > 0) {   //Determine which of trailing's children n is using stringComp value
                n->parent->right = n;
                
            } else if (stringComp < 0) {
                n->parent->left = n;
                
            }
            tld->count++;
            rebalance(tld, trailing);   //Rebalance node trailing (n->parent)
            
            return 1;
            
        } else {
            return 0;

        }
    }
}


/*Returns total amount of log entries in AVLTree*/
long tldlist_count(TLDList *tld) {
	if (tld != NULL) {
		return tld->count;
		
	} else {
		return 0;
		
	}
}


/*Creates a new TLDList iterator starting at min(tld->root) for inorder traversal*/
TLDIterator *tldlist_iter_create(TLDList *tld) {
	if (tld != NULL) {
		TLDIterator *newIt;
    
		if ((newIt = (struct tlditerator *)malloc(sizeof(struct tlditerator))) != NULL) {
			newIt->max  = TLDMax(tld->root);    //Max used to limit succession to size of tree
			newIt->node = TLDMin(tld->root);    //Start iteration at minimum node (inorder traversal)
		}
		
		return newIt;
		
    } else {
		return NULL;
		
	}
}


/*Iterates TLDList, iterator remains 'one step ahead' of return value*/
TLDNode *tldlist_iter_next(TLDIterator *iter) {
    if (iter != NULL) {  //Ensure iterator creation was successful
        TLDNode *temp = iter->node; //Store current iter->node for return value
        iter->node = (temp == iter->max || !temp) ? NULL : succ(iter->node);    //Update iter node, set to NULL if current is max value, successor of node otherwise

        return temp;

    } else {
        return NULL;     //Iterator creation unsuccessful

    }
}


/*Destroy iter*/
void tldlist_iter_destroy(TLDIterator *iter) {
	if (iter != NULL) { //Ensure iterator exists and set all pointers to NULL before freeing it
		iter->max = NULL;
		iter->node = NULL;
		free(iter);
	}
}


/*Returns name of TLDNode node*/
char *tldnode_tldname(TLDNode *node) {
    if (node != NULL) {
        return node->name;

    } else {
		return NULL;
		
    }
}


/*Returns total count of TLDNode node*/
long tldnode_count(TLDNode *node) {
	if (node != NULL) {
        return node->total;

	} else {
        return 0;
		
    }
}


/* --- ADDITIONAL FUNCTIONS --- */

/*Gets the TLD of a URL (For naming a node)*/
char *getLabel(char *p, char *c) {
    return (c == NULL) ? p : getLabel(c, strtok(NULL, "."));
}


/*Rebalances a node*/
void rebalance(TLDList *tld, TLDNode *p) {
    SETBALANCE(p);  //Set p->balance
    
    p = (p->balance == -2) ? //Left subtree too large, determine which of p->left's subtree's is largest and rotate accordingly
            ( (GETHEIGHT(p->left->left)   >= GETHEIGHT(p->left->right)) ? rotateRight(p) : rotateLeftThenRight(p) ) :
            
        (p->balance ==  2) ? //Right subtree too large, determine which of p->right's subtree's is largest and rotate accordingly
            ( (GETHEIGHT(p->right->right) >= GETHEIGHT(p->right->left)) ? rotateLeft(p)  : rotateRightThenLeft(p) ) : p; //"else p = p"
    
    if (p->parent != NULL) {
        rebalance(tld, p->parent);
        
    } else {
        tld->root = p;

    }
}


/*Returns minimum node in subtree of n*/
TLDNode *TLDMin(TLDNode *n) {
	if (n != NULL) {
		while (n->left != NULL) {
			n = n->left;
		}
		
		return n;
		
	} else {
		return NULL;
		
	}
}


/*Returns maximum node in subtree of n*/
TLDNode *TLDMax(TLDNode *n) {
	if (n != NULL) {
		while (n->right != NULL) {
			n = n->right;
		}
		
		return n;
		
	} else {
		return NULL;
		
	}
}


/*Rotates n left*/
TLDNode *rotateLeft(TLDNode *n) {
    TLDNode *nRight = n->right; //Get right child of n
    
    nRight->parent = n->parent; //Set right child's parent to n's parent
    n->right = nRight->left;    //Set n's right child to the right child's left child
    
    if (n->right != NULL) {     //Update new right child's parent if needed
        n->right->parent = n;
    }
    
    nRight->left = n;           //Set n to nRight's left child
    n->parent = nRight;         //Update parent 
    
    if (nRight->parent != NULL) {
        if (nRight->parent->right == n) {
            nRight->parent->right = nRight;
            
        } else {
            nRight->parent->left = nRight;
            
        }
    }
    
    SETBALANCE(n);
    SETBALANCE(nRight);
    
    return nRight;
}


/*Rotates n right*/
TLDNode *rotateRight(TLDNode *n) {
    TLDNode *nLeft = n->left;   //Get left child
    
    
    nLeft->parent = n->parent;  //Set left child's parent to n's parent
    n->left = nLeft->right;     //Set n's left child to the left child's left child
    
    if (n->left != NULL) {      //Update new left child's parent if needed
        n->left->parent = n;
    }
    
    nLeft->right = n;           //Set n to nLeft's left child
    n->parent = nLeft;          //Update parent 
    
    if (nLeft->parent != NULL) {
        if (nLeft->parent->right == n) {
            nLeft->parent->right = nLeft;
            
        } else {
            nLeft->parent->left = nLeft;
            
        }
    }
    
    SETBALANCE(n);
    SETBALANCE(nLeft);
    
    return nLeft;
}


/*Rotates n->left left then n right*/
TLDNode *rotateLeftThenRight(TLDNode *n) {
    n->left = rotateLeft(n->left);
    return rotateRight(n);
}


/*Rotates n->right right then n left*/
TLDNode *rotateRightThenLeft(TLDNode *n) {
    n->right = rotateRight(n->right);
    return rotateLeft(n);
}


/*Return successor of node n, for inorder traversal*/
TLDNode *succ(TLDNode *n) {
	if (n != NULL) {
		if (n->right != NULL) {      //If right subtree of n exists return min(n->right)
			return TLDMin(n->right);
		}
		
		TLDNode *trailing = n;
		TLDNode *leading  = trailing->parent;
		
		while (leading != NULL && leading->right == trailing) {	//While leading isn't root and leading's right child is trailing
			trailing = leading;
			leading = trailing->parent;	//Update values to 'climb' tree
		}
		
		return leading;
		
	} else {
		return NULL;
		
	}
}


/*Create a new TLDNode with name = nodeLabel and return it, return NULL upon failure*/
TLDNode *createNewNode(char *nodeLabel) {
    if (nodeLabel != NULL && strcmp(nodeLabel, "") != 0) { //Check that nodeLabel exists
        TLDNode *n;

        if ((n = (struct tldnode *)malloc(sizeof(struct tldnode))) != NULL) {
            n->name = (char *)malloc(sizeof(nodeLabel) + 1);	//Add 1 for '\0'
            strcpy(n->name, nodeLabel);
        
            n->parent  = n->left   = n->right = NULL;
            n->balance = n->height = n->total = 0;
        }

        return n;
		
    } else {
		return NULL;
		
	}
}

