#ifndef _XML_H
#define _XML_H

#include <stdbool.h>

struct Attribute {
	char * key;
	char * value;
};

struct Tag {
	char * name;
	int * attributes;
	size_t attributes_size;
	size_t attributes_cap;
	int * items;
	size_t items_size;
	size_t items_cap;
};

struct Document {
	struct Tag * tags;
	size_t tags_size;
	size_t tags_cap;
	struct Attribute * attributes;
	size_t attributes_size;
	size_t attributes_cap;
	char ** strings;
	size_t strings_size;
	size_t strings_cap;
};

enum XML_Err {
	XML_OK,
	XML_BUILD_ERRORS = 0x0000,
	XML_OUT_OF_RANGE,
	XML_CONSECUATIVE_STRINGS,
	XML_ALLOCATION_ERROR,
	XML_PARSE_ERRORS = 0x1000,
};

struct Document * new_document();

struct Document * parse_document(const char * string, XML_Err * err);

bool is_tag(struct Tag * tag, size_t index);

bool is_string(struct Tag * tag, size_t index);

void * get_item(struct Tag * tag, size_t index);

struct Tag * get_tag(struct Tag * tag, size_t index);

char * get_string(struct Tag * tag, size_t index);

XML_Err append_tag(struct Tag * tag, struct Tag * new_tag);

XML_Err append_string(struct Tag * tag, char * new_string);

XML_Err prepend_tag(struct Tag * tag, struct Tag * new_tag);

XML_Err prepend_string(struct Tag * tag, char * new_string);

XML_Err insert_tag(struct Tag * tag, struct Tag * new_tag, size_t index);

XML_Err insert_string(struct Tag * tag, char * new_string, size_t index);

#endif