#ifndef SLIST_h
#define SLIST_h 1

typedef struct slist_s slist_t;
struct slist_s {
	slist_t *next;
};

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, member) ((size_t)&((type*)0)->member)
#endif
#define slist_entry(node, type, member) \
	(type*)(((char*)(node)) - FIELD_OFFSET(type, member))

#define slist_init_head(head) \
	((head)->next = NULL)

#define slist_add(node,head) \
	((node)->next = (head)->next, \
	 (head)->next = (node))

#define slist_del(node,prev) \
	((prev)->next = (node)->next)

#define slist_empty(head) \
	((head)->next == NULL)

#define slist_for_each_entry(pos, head, type, member) \
	for (pos = slist_entry((head)->next, type, member); \
		&pos->member != NULL; \
		pos = slist_entry(pos->member.next, type, member))

#endif
