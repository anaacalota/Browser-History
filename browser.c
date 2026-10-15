#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1024
#define MAX_PAGES 1000

// Represents a web page
typedef struct {
	int id;
	char url[50];
	char *description;
} page_t;

// Represents a node in a page list
typedef struct page_node_t page_node_t;
struct page_node_t {
	page_node_t *next;
	page_t *page;
};

// Represents a page list
typedef struct {
	page_node_t *head;
	unsigned int size;
} page_list_t;

// A stack backed by a page list
typedef struct {
	page_list_t *page_list;
} stack_t;

// Represents a browser tab
typedef struct {
	int id;
	page_t *current_page;
	stack_t *backward_stack;
	stack_t *forward_stack;
} tab_t;

// Represents a node in the tab list
typedef struct tab_node_t tab_node_t;
struct tab_node_t {
	tab_t *tab;
	tab_node_t *next;
	tab_node_t *prev;
};

// Stores the browser's open tabs
typedef struct {
	tab_node_t *head;
	unsigned int size;
} tabs_list_t;

// Represents the browser
typedef struct {
	tab_t *current;
	tabs_list_t *tabs_list;
} browser_t;

// Creates a page with the given ID, URL, and description
page_t *create_page(int id, char *url, char *description)
{
	page_t *page = (page_t *)malloc(sizeof(*page));
	if (!page) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	page->id = id;

	strncpy(page->url, url, sizeof(page->url) - 1);
	page->url[sizeof(page->url) - 1] = '\0';

	size_t len = strlen(description) + 1; // Include the null terminator
	page->description = malloc(len);
	if (!page->description) {
		fprintf(stderr, "malloc failed\n");
		free(page);
		return NULL;
	}

	memcpy(page->description, description, len);

	return page;
}

// Creates an empty page list
page_list_t *create_page_list(void)
{
	page_list_t *list = (page_list_t *)malloc(sizeof(*list));
	if (!list) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	list->head = NULL;
	list->size = 0;

	return list;
}

// Creates a node for a page
page_node_t *create_page_node(page_t *page)
{
	page_node_t *node = (page_node_t *)malloc(sizeof(*node));
	if (!node) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	node->next = NULL;
	node->page = page;

	return node;
}

// Creates a stack used to store page history
stack_t *create_stack(void)
{
	stack_t *stack = (stack_t *)malloc(sizeof(*stack));
	if (!stack) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	stack->page_list = create_page_list();
	if (!stack->page_list) {
		free(stack);
		return NULL;
	}

	return stack;
}

// Creates an empty tab list
tabs_list_t *create_tabs_list(void)
{
	tabs_list_t *list = (tabs_list_t *)malloc(sizeof(*list));
	if (!list) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	list->head = NULL;
	list->size = 0;

	return list;
}

// Creates a node for a tab
tab_node_t *create_tab_node(tab_t *tab)
{
	tab_node_t *node = (tab_node_t *)malloc(sizeof(*node));
	if (!node) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	node->tab = tab;
	node->next = NULL;
	node->prev = NULL;

	return node;
}

void free_stack(stack_t *stack);

// Creates a tab associated with the given page
tab_t *create_tab(int id, page_t *page)
{
	tab_t *tab = (tab_t *)malloc(sizeof(*tab));
	if (!tab) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	tab->id = id;
	tab->current_page = page;

	tab->backward_stack = create_stack();
	if (!tab->backward_stack) {
		free(tab);
		return NULL;
	}

	tab->forward_stack = create_stack();
	if (!tab->forward_stack) {
		free_stack(tab->backward_stack);
		free(tab);
		return NULL;
	}

	return tab;
}

// Memory cleanup functions
void free_page(page_t *page)
{
	if (!page)
		return;

	free(page->description);
	page->description = NULL;

	free(page);
}

void free_page_node(page_node_t *node)
{
	if (!node)
		return;

	free(node);
}

void free_page_list(page_list_t *list)
{
	if (!list)
		return;

	page_node_t *node = list->head;
	while (node) {
		page_node_t *next = node->next;
		free_page_node(node);
		node = next;
	}

	free(list);
	list = NULL;
}

void st_pop(stack_t *st);
int st_get_size(stack_t *stack);

void free_stack(stack_t *stack)
{
	if (!stack)
		return;

	while (st_get_size(stack))
		st_pop(stack);

	free_page_list(stack->page_list);
	stack->page_list = NULL;

	free(stack);
	stack = NULL;
}

void free_tab(tab_t *tab)
{
	if (!tab)
		return;

	free_stack(tab->backward_stack);
	tab->backward_stack = NULL;

	free_stack(tab->forward_stack);
	tab->forward_stack = NULL;

	free(tab);
	tab = NULL;
}

void free_tab_node(tab_node_t *node)
{
	if (!node)
		return;

	free_tab(node->tab);

	free(node);
	node = NULL;
}

void free_tabs_list(tabs_list_t *list)
{
	if (!list)
		return;

	tab_node_t *node = list->head;
	for (unsigned int i = 0; node && i < list->size; i++) {
		tab_node_t *next = node->next;
		free_tab_node(node);
		node = next;
	}

	free(list);
	list = NULL;
}

void free_browser(browser_t *browser)
{
	if (!browser)
		return;

	if (browser->tabs_list)
		free_tabs_list(browser->tabs_list);

	free(browser);
	browser = NULL;
}

// Creates the browser and adds its default tab
browser_t *create_browser(tab_t *default_tab)
{
	browser_t *browser = (browser_t *)malloc(sizeof(*browser));
	if (!browser) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	browser->tabs_list = create_tabs_list();
	if (!browser->tabs_list) {
		free(browser);
		return NULL;
	}

	tab_node_t *default_node = (tab_node_t *)malloc(sizeof(tab_node_t));
	if (!default_node) {
		free(browser->tabs_list);
		free(browser);
		return NULL;
	}

	default_node->next = default_node;
	default_node->prev = default_node;
	default_node->tab = default_tab;
	browser->tabs_list->head = default_node;
	default_node->next = browser->tabs_list->head;
	default_node->prev = browser->tabs_list->head;

	browser->tabs_list->head->next = default_node;
	browser->tabs_list->head->prev = default_node;
	browser->tabs_list->size = 1;
	browser->current = default_tab;

	return browser;
}

// Returns the nth node in the tab list
tab_node_t *get_nth_node(tabs_list_t *list, int n)
{
	if (!list || !list->size)
		return NULL;

	tab_node_t *node = list->head;
	for (int i = 0; i < n; i++)
		node = node->next;

	return node;
}

// Pushes a page onto a stack
void st_push(stack_t *st, page_t *page)
{
	if (!st || !page)
		return;

	page_node_t *node = create_page_node(page);
	if (!node)
		return;

	page_node_t *top_node = st->page_list->head;
	page_node_t *prev_top_node = NULL;

	while (top_node) {
		prev_top_node = top_node;
		top_node = top_node->next;
	}

	if (!prev_top_node)
		st->page_list->head = node;
	else
		prev_top_node->next = node;

	node->next = NULL;
	st->page_list->size++;
}

// Adds a new tab to the browser
void create_new_tab(browser_t *browser, page_t *page, page_t *default_page,
					tab_node_t *default_tab_node)
{
	if (!browser || !page)
		return;

	tab_node_t *new_tab = default_tab_node;
	tab_node_t *prev_tab;

	if (browser->tabs_list->head) {
		prev_tab = get_nth_node(browser->tabs_list, browser->tabs_list->size
				   - 1);
		new_tab->prev = prev_tab;
		new_tab->next = prev_tab->next;
		prev_tab->next->prev = new_tab;
		prev_tab->next = new_tab;
	}

	browser->current = new_tab->tab;
	browser->current->current_page = default_page;
	browser->tabs_list->size++;
}

// Finds a tab by its ID
tab_node_t *get_tab_by_id(tabs_list_t *list, int id)
{
	if (!list || !list->size)
		return NULL;

	tab_node_t *start = list->head;
	tab_node_t *node = start;

	do {
		if (node->tab->id == id)
			return node;
		node = node->next;
	} while (node != start);

	return NULL;
}

// Closes a tab in the browser
void close_tab(browser_t *browser, int id, FILE *output)
{
	if (!browser || !browser->tabs_list || browser->tabs_list->size == 0)
		return;

	tab_node_t *removed = get_tab_by_id(browser->tabs_list, id);
	if (!removed)
		return;

	if (removed == browser->tabs_list->head) {
		fprintf(output, "403 Forbidden\n");
		return;
	}

	removed->next->prev = removed->prev;
	removed->prev->next = removed->next;
	browser->tabs_list->size--;
	browser->current = removed->prev->tab;

	free_tab_node(removed);
}

// Returns the number of pages in a stack
int st_get_size(stack_t *stack)
{
	if (!stack)
		return 0;

	return stack->page_list->size;
}

// Checks whether a stack is empty
unsigned int st_is_empty(stack_t *st)
{
	if (!st)
		return 1;

	return st_get_size(st) == 0;
}

// Returns the top page without removing it
void *st_peek(stack_t *st)
{
	if (!st || st_is_empty(st))
		return NULL;

	page_node_t *node = st->page_list->head;

	for (int i = 0; i < st_get_size(st) - 1; i++)
		node = node->next;

	if (!node)
		return NULL;

	return node->page;
}

// Pops the top page from a stack
void st_pop(stack_t *st)
{
	if (!st || st_is_empty(st))
		return;

	page_node_t *node = st->page_list->head;
	page_node_t *prev_node = NULL;

	while (node->next) {
		prev_node = node;
		node = node->next;
	}

	if (prev_node)
		prev_node->next = NULL;
	else
		st->page_list->head = NULL;

	st->page_list->size--;
	free_page_node(node);
}

// Opens a file
FILE *open_file(const char *filename, const char *mode)
{
	FILE *file = fopen(filename, mode);
	if (!file) {
		fprintf(stderr, "fopen failed\n");
		return NULL;
	}

	return file;
}

// Initializes the page array from the input file
page_t **initialize_pages(FILE *file, int nr_pages)
{
	page_t **pages = (page_t **)malloc(nr_pages * sizeof(page_t *));
	if (!pages) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	for (int i = 0; i < nr_pages; i++) {
		int id;
		char url[50];
		char *description = (char *)malloc(MAX * sizeof(char));
		if (!description) {
			fprintf(stderr, "malloc failed\n");
			for (int j = 0; j < i; j++)
				free_page(pages[j]);
			free(pages);
			return NULL;
		}

		fscanf(file, "%d\n", &id);
		fgets(url, sizeof(url), file);
		url[strcspn(url, "\n")] = 0;

		fgets(description, MAX, file);
		description[strcspn(description, "\n")] = 0;

		pages[i] = create_page(id, url, description);
		if (!pages[i]) {
			fprintf(stderr, "create_page failed\n");
			free(description);
			for (int j = 0; j < i; j++)
				free_page(pages[j]);
			free(pages);
			return NULL;
		}
		free(description);
	}
	return pages;
}

// Allocates a generic array of pointers
void **initialize_list(size_t nr_elements, size_t element_size)
{
	void **list = (void **)malloc(nr_elements * element_size);
	if (!list) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}

	return list;
}

int handle_new_tab_command(browser_t *browser, page_t **pages, int nr_pages,
						   page_t **default_pages_list,
						   tab_node_t **default_tabs_list,
						   int *default_pages_count, int *next_tab_id)
{
	if (*default_pages_count == MAX_PAGES) {
		fprintf(stderr, "Maximum number of default pages reached\n");
		return -1;
	}

	default_pages_list[*default_pages_count] = create_page(0, "https://acs.pub.ro/", "Computer Science");
	if (!default_pages_list[*default_pages_count]) {
		fprintf(stderr, "create_page failed\n");
		return -1;
	}

	tab_t *new_tab = create_tab(*next_tab_id,
								default_pages_list[*default_pages_count]);
	if (!new_tab) {
		free_page(default_pages_list[*default_pages_count]);
		default_pages_list[*default_pages_count] = NULL;
		return -1;
	}
	default_tabs_list[*default_pages_count] = create_tab_node(new_tab);
	if (!default_tabs_list[*default_pages_count]) {
		fprintf(stderr, "create_tab_node failed\n");
		free_tab(new_tab);
		free_page(default_pages_list[*default_pages_count]);
		default_pages_list[*default_pages_count] = NULL;
		return -1;
	}
	(*default_pages_count)++;

	if (*next_tab_id - 1 < nr_pages) {
		create_new_tab(browser, pages[*next_tab_id - 1],
					   default_pages_list[*default_pages_count - 1],
					   default_tabs_list[*default_pages_count - 1]);
	} else {
		create_new_tab(browser, browser->current->current_page,
					   default_pages_list[*default_pages_count - 1],
					   default_tabs_list[*default_pages_count - 1]);
	}

	(*next_tab_id)++;

	return 0;
}

void handle_close_tab(browser_t *browser, FILE *output)
{
	if (browser->tabs_list->size == 1) {
		fprintf(output, "403 Forbidden\n");
		return;
	}
	close_tab(browser, browser->current->id, output);
}

void handle_open_tab(browser_t *browser, FILE *file, FILE *output)
{
	int id;
	fscanf(file, "%d", &id);
	tab_node_t *tab_node = get_tab_by_id(browser->tabs_list, id);
	if (!tab_node) {
		fprintf(output, "403 Forbidden\n");
		return;
	}
	browser->current = tab_node->tab;
}

void handle_next_tab(browser_t *browser)
{
	tab_node_t *next_tab = get_tab_by_id(browser->tabs_list,
										 browser->current->id);
	if (next_tab)
		browser->current = next_tab->next->tab;
}

void handle_prev_tab(browser_t *browser)
{
	tab_node_t *prev_tab = get_tab_by_id(browser->tabs_list,
										 browser->current->id);
	if (prev_tab)
		browser->current = prev_tab->prev->tab;
}

void handle_page_command(browser_t *browser, FILE *file, FILE *output,
						 page_t **pages, int nr_pages)
{
	if (nr_pages == 0) {
		fprintf(output, "403 Forbidden\n");
		return;
	}

	int id;
	fscanf(file, "%d", &id);
	int page_index = -1;

	// Find the requested page by its ID
	for (int i = 0; i < nr_pages; i++) {
		if (pages[i]->id == id) {
			page_index = i;
			break;
		}
	}

	if (page_index == -1) {
		fprintf(output, "403 Forbidden\n");
		return;
	}

	st_push(browser->current->backward_stack, browser->current->current_page);

	browser->current->current_page = pages[page_index];

	while (!st_is_empty(browser->current->forward_stack))
		st_pop(browser->current->forward_stack);
}

void handle_backward_command(browser_t *browser, FILE *output)
{
	if (st_is_empty(browser->current->backward_stack)) {
		fprintf(output, "403 Forbidden\n");
		return;
	}

	if (browser->current->current_page->id == 0) {
		st_push(browser->current->backward_stack,
				browser->current->current_page);
	}

	st_push(browser->current->forward_stack, browser->current->current_page);

	page_t *prev_page = st_peek(browser->current->backward_stack);
	browser->current->current_page = prev_page;

	st_pop(browser->current->backward_stack);
}

void handle_forward_command(browser_t *browser, FILE *output)
{
	if (st_is_empty(browser->current->forward_stack)) {
		fprintf(output, "403 Forbidden\n");
		return;
	}

	st_push(browser->current->backward_stack, browser->current->current_page);

	page_t *next_page = st_peek(browser->current->forward_stack);
	browser->current->current_page = next_page;

	st_pop(browser->current->forward_stack);
}

void handle_print_history_command(browser_t *browser, FILE *file, FILE *output)
{
	int id;
	fscanf(file, "%d", &id);

	tab_node_t *tab_node = get_tab_by_id(browser->tabs_list, id);
	if (!tab_node) {
		fprintf(output, "403 Forbidden\n");
		return;
	}

	stack_t *forward = tab_node->tab->forward_stack;
	if (!st_is_empty(forward)) {
		page_node_t *node = forward->page_list->head;
		while (node) {
			fprintf(output, "%s\n", node->page->url);
			node = node->next;
		}
	}

	fprintf(output, "%s\n", tab_node->tab->current_page->url);

	stack_t *backward = tab_node->tab->backward_stack;
	if (!st_is_empty(backward)) {
		char **history = (char **)malloc(backward->page_list->size *
										 sizeof(char *));
		if (!history) {
			fprintf(stderr, "Memory allocation failed for history\n");
			return;
		}

		int index = 0;
		page_node_t *node = backward->page_list->head;
		while (node) {
			history[index++] = node->page->url;
			node = node->next;
		}

		for (int i = index - 1; i >= 0; i--)
			fprintf(output, "%s\n", history[i]);

		free(history);
	}
}

void handle_print_command(browser_t *browser, FILE *output)
{
	tab_node_t *curr_node = browser->tabs_list->head;

	while (curr_node->tab != browser->current)
		curr_node = curr_node->next;

	tab_node_t *start_node = curr_node;
	do {
		fprintf(output, "%d ", curr_node->tab->id);
		curr_node = curr_node->next;
	} while (curr_node != start_node);

	fprintf(output, "\n");

	fprintf(output, "%s\n", browser->current->current_page->description);
}

void cleanup_resources(char *command, page_t **pages, int nr_pages,
					   page_t *default_page,
					   page_t **default_pages_list,
					   tab_node_t **default_tabs_list,
					   int default_pages_count, browser_t *browser,
					   FILE *file, FILE *output)
{
	free(command);

	for (int i = 0; i < nr_pages; i++)
		free_page(pages[i]);

	free(pages);

	free_page(default_page);
	for (int i = 0; i < default_pages_count; i++)
		free_page(default_pages_list[i]);

	free(default_tabs_list);
	free(default_pages_list);

	free_browser(browser);

	fclose(file);
	fclose(output);
}

int main(void)
{
	FILE *file = open_file("tema1.in", "r");
	FILE *output = open_file("tema1.out", "w");
	if (!file || !output) {
		if (file)
			fclose(file);
		if (output)
			fclose(output);
		return -1;
	}
	int nr_pages, nr_ops, next_tab_id = 1, default_pages_count = 0;

	char *command = (char *)malloc(MAX * sizeof(char));
	if (!command) {
		fprintf(stderr, "malloc failed\n");
		fclose(file);
		fclose(output);
		return -1;
	}

	fscanf(file, "%d\n", &nr_pages);

	page_t **pages = initialize_pages(file, nr_pages);
	page_t **default_pages_list = (page_t **)initialize_list
								  (MAX_PAGES, sizeof(page_t *));
	tab_node_t **default_tabs_list = (tab_node_t **)initialize_list
									 (MAX_PAGES, sizeof(tab_node_t *));
	if (!pages || !default_pages_list || !default_tabs_list) {
		free(command);
		free(pages);
		free(default_pages_list);
		free(default_tabs_list);
		if (file)
			fclose(file);
		if (output)
			fclose(output);
		return -1;
	}

	fscanf(file, "%d", &nr_ops);

	browser_t *browser = NULL;
	page_t *default_page = create_page(0, "https://acs.pub.ro/",
									   "Computer Science");
	if (!default_page) {
		free(command);
		for (int i = 0; i < nr_pages; i++)
			free_page(pages[i]);
		free(pages);
		free(default_pages_list);
		free(default_tabs_list);
		fclose(file);
		fclose(output);
		return -1;
	}
	tab_t *default_tab = create_tab(0, default_page);
	if (!default_tab) {
		free_page(default_page);
		free(command);
		for (int i = 0; i < nr_pages; i++)
			free_page(pages[i]);
		free(pages);
		free(default_pages_list);
		free(default_tabs_list);
		fclose(file);
		fclose(output);
		return -1;
	}
	browser = create_browser(default_tab);
	if (!browser) {
		fprintf(stderr, "create_browser failed\n");
		free_tab(default_tab);
		free_page(default_page);
		for (int i = 0; i < nr_pages; i++)
			free_page(pages[i]);
		free(pages);
		free(default_pages_list);
		free(default_tabs_list);
		free(command);
		fclose(file);
		fclose(output);
		return -1;
	}

	for (int i = 0; i < nr_ops; i++) {
		fscanf(file, "%s", command);

		if (!strcmp(command, "NEW_TAB")) {
			handle_new_tab_command(browser, pages, nr_pages,
								   default_pages_list, default_tabs_list,
								   &default_pages_count, &next_tab_id);
		} else if (!strcmp(command, "CLOSE")) {
			handle_close_tab(browser, output);
		} else if (!strcmp(command, "OPEN")) {
			handle_open_tab(browser, file, output);
		} else if (!strcmp(command, "NEXT")) {
			handle_next_tab(browser);
		} else if (!strcmp(command, "PREV")) {
			handle_prev_tab(browser);
		} else if (!strcmp(command, "PAGE")) {
			handle_page_command(browser, file, output, pages, nr_pages);
		} else if (!strcmp(command, "BACKWARD")) {
			handle_backward_command(browser, output);
		} else if (!strcmp(command, "FORWARD")) {
			handle_forward_command(browser, output);
		} else if (!strcmp(command, "PRINT_HISTORY")) {
			handle_print_history_command(browser, file, output);
		} else if (!strcmp(command, "PRINT")) {
			handle_print_command(browser, output);
		}
	}
	cleanup_resources(command, pages, nr_pages, default_page,
					  default_pages_list, default_tabs_list,
					  default_pages_count, browser, file, output);
	return 0;
}
