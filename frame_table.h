#ifndef FRAME_TABLE_H
#define FRAME_TABLE_H

struct frame_table;

// Create frame table
struct frame_table* frame_table_create(int nframes);

// Delete frame table
struct frame_table* frame_table_delete(struct frame_table* ft);

// Read <id> from frame table
int frame_table_read(struct frame_table* ft, int frame_id);

// Set <id> to <free> in frame table
void frame_table_set(struct frame_table* ft, int frame_id, int page_id);

// Returns <id> of next free frame
int get_next_free(struct frame_table* ft);

// Returns <id> of next frame to replace
int get_next_replacement(struct frame_table* ft, int replacement_strategy);

// Print frame table
void frame_table_print(struct frame_table* ft);

#endif