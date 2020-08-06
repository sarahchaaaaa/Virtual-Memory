#include <stdio.h>
#include <stdlib.h>

// Frame Table - contains list of <nframes> frame -> page_id mappings
// frame_id -> page_id (or -1 if no page present)
struct frame_table {
    int nframes;
    int* page_id;  // page_id[frame_id] = page_id the frame holds
    int* fifo_bit; // Bit for fifo. 1 = first in
    int* lru_bit;  // Bit for lru. 1 = least recently used
};

// Create frame table
struct frame_table* frame_table_create(int nframes) {
    struct frame_table* ft = malloc(sizeof(struct frame_table));
    ft->nframes = nframes;
    ft->page_id = malloc(sizeof(int) * ft->nframes);
    ft->fifo_bit = malloc(sizeof(int) * ft->nframes);
    ft->lru_bit  = malloc(sizeof(int) * ft->nframes);

    int i;
    for (i = 0; i < ft->nframes; ++i) {
        ft->page_id[i] = -1;
    }

    return ft;
}

// Delete frame table
void frame_table_delete(struct frame_table* ft) {
    free(ft->page_id);
    free(ft->fifo_bit);
    free(ft->lru_bit);
    free(ft);
    return;
}

// Read page_id at <frame_id> from frame table
int frame_table_read(struct frame_table* ft, int frame_id) {
    return ft->page_id[frame_id];
}

// Set page_id at <frame_id> in frame table
void frame_table_set(struct frame_table* ft, int frame_id, int page_id) {
    ft->page_id[frame_id] = page_id;

    // Set fifo bits
    int i;
    for (i = 0; i < ft->nframes; ++i) {
        // Set this frame's fifo bit to 1 (first in!)
        if (i == frame_id) ft->fifo_bit[i] = 1;
        else {
            // If no frame, keep fifo bit to 0 (no frame)
            if (ft->page_id[i] == -1) ft->fifo_bit[i] = 0;

            // Otherwise, increment fifo bit
            else ft->fifo_bit[i] += 1;
        }
    }

    // Set lru bits
    for (i = 0; i < ft->nframes; ++i) {
        // Set this frame's lru bit to 1 (lowest)
        if (i == frame_id) ft->lru_bit[i] = 1;

        else {
            // If no frame, keep lru bit to 0 (no frame)
            if (ft->page_id[i] == -1) ft->lru_bit[i] = 0;

            // If lru bit = nframes, can't go higher! (highest)
            else if (ft->lru_bit[i] == ft->nframes) ;

            // Otherwise, increment lru bit
            else ft->lru_bit[i] += 1;
        }
    }
    return;
}

// Returns <frame_id> of next free frame, or -1 if no free frame
int get_next_free(struct frame_table* ft) {
    int free_frame_id = -1;
    int i;
    for (i = 0; i < ft->nframes; ++i) {
        if (ft->page_id[i] == -1) {
            free_frame_id = i;
            break;
        }
    }
    return free_frame_id;
}

// Returns <frame_id> of next frame to replace, according
// to either <rand, fifo, lru>
int get_next_replacement(struct frame_table* ft, int replacement_strategy) {
    int frame_id = -1;
    // rand
    if (replacement_strategy == 0) {
        frame_id = rand() % ft->nframes;
    }
    // fifo
    else if (replacement_strategy == 1) {
        int i;
        for (i = 0; i < ft->nframes; ++i) {
            if (ft->fifo_bit[i] == ft->nframes) frame_id = i;
            if (ft->fifo_bit[i] > ft->nframes) {
                printf("frame_table.c error: fifo_bit %d above nframes (%d)", i, ft->nframes);
                exit(1);
            }
        }
    }
    // lru
    else if (replacement_strategy == 2) {
        int i;
        for (i = 0; i < ft->nframes; ++i) {
            if (ft->lru_bit[i] == 1) frame_id = i;
        }
    }

    return frame_id;
}

// Print frame table
void frame_table_print(struct frame_table* ft) {
    int i;
    printf("frame_id\tpage_id\t\tfifo_bit\tlru_bit\n");
    for (i = 0; i < ft->nframes; ++i) {
        printf("%d\t\t%d\t\t%d\t\t%d\n", i, ft->page_id[i], ft->fifo_bit[i], ft->lru_bit[i]);
    }
    printf("\n");
    return;
}
