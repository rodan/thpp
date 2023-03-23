
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <filesystem>
#include <algorithm>
#include "imgui.h"
#include "proj.h"
#include "opengl_helper.h"
#include "viewport.h"
#include "main_cli.h"
#include "version.h"
#include "file_library.h"

#define          FNAME_MAX  256
//possible flags
#define      FL_FILE_READY  0x1
#define    FL_FILE_INVALID  0x2
#define    FL_FILE_PREPARE  0x4

uint32_t file_tx;
uint32_t dir_tx;

struct node {
    uint16_t flags;
    char fname[FNAME_MAX];
    struct stat st;
    uint32_t texture;
    uint16_t width;
    uint16_t height;
    struct node *next;
};
typedef struct node node_t;

node_t *head = NULL;

th_db_t thumb;

float thumbnail_size = 128.0f;

void ll_print(node_t * head);
node_t *ll_find_tail(node_t * head);
node_t *ll_add(node_t ** head, const uint8_t val);
node_t *ll_remove(node_t ** head, const uint8_t val);

void ll_print(node_t * head)
{
    node_t *p = head;

    if (head == NULL) {
        printf("ll is empty\n");
        return;
    }

    while (NULL != p) {
        printf("n %p, [%s]  next %p\n", (void *)p, p->fname, (void *)p->next);
        if (p->next != NULL) {
            p = p->next;
        } else {
            return;
        }
    }
}

node_t *ll_find_tail(node_t * head)
{
    node_t *p = head;

    while (NULL != p) {
        if (p->next != NULL) {
            p = p->next;
        } else {
            return (p);
        }
    }

    return (p);
}

void ll_free_all(node_t ** head)
{
    node_t *p = *head;
    node_t *del;

    while (NULL != p) {
        //printf("remove node @%p\n", (void *)p);
        del = p;
        if (del->texture) {
            free_textures(1, &del->texture);
        }
        p = p->next;
        free(del);
    }

    *head = NULL;
}

node_t *ll_add(node_t ** head)
{
    node_t *p = ll_find_tail(*head);
    node_t *new_node = NULL;

    new_node = (node_t *) calloc(1, sizeof(node_t));
    if (new_node == NULL) {
        printf("malloc error\n");
        return NULL;
    } else {
        //printf("new node @%p\n", (void *)new_node);
    }

    new_node->next = NULL;
    //new_node->value = val;

    if (p == NULL) {
        *head = new_node;
        //printf("new head @%p\n", (void *)new_node);
    } else {
        p->next = new_node;
        //printf("new node added to @%p\n", (void *)p);
    }

    return new_node;
}

void file_library_init(void)
{
    unsigned int w, h;
    file_tx = 0;
    dir_tx = 0;

    if (load_texture_from_file("res/file_icon.png", &file_tx, &w, &h) != EXIT_SUCCESS) {
        fprintf(stderr, "error loading file icon\n");
    }
    if (load_texture_from_file("res/dir_icon.png", &dir_tx, &w, &h) != EXIT_SUCCESS) {
        fprintf(stderr, "error loading directory icon\n");
    }
}

void file_library_free(void)
{
    ll_free_all(&head);
    cleanup(&thumb);
}

std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),[](unsigned char c) { return std::tolower(c);} // correct
    );
    return s;
}

uint8_t node_populate(node_t * node, const char *abs_path)
{
    if (stat(abs_path, &node->st) < 0) {
        node->flags |= FL_FILE_INVALID;
        return EXIT_FAILURE;
    }

    cleanup(&thumb);

    if (thumb.p.in_file) {
        free(thumb.p.in_file);
    }

    thumb.p.in_file = (char *)calloc(strlen(abs_path) + 1, sizeof(char));
    strncpy(thumb.p.in_file, abs_path, strlen(abs_path));
    thumb.p.pal = 6;
    thumb.p.zoom = 1;

    if (main_cli(&thumb, 0) == EXIT_SUCCESS) {
        load_texture_from_mem(thumb.rgba.data, &node->texture, thumb.rgba.width, thumb.rgba.height);
        //printf("tex %u for %s\n", node->texture, abs_path);
        node->width = thumb.rgba.width;
        node->height = thumb.rgba.height;
        node->flags = FL_FILE_READY;
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "warning: %s can't be opened as a thermal image\n", abs_path);
        node->flags = FL_FILE_INVALID;
    }

    return EXIT_FAILURE;
}

node *node_search_fname(const char *fname)
{
    node_t *search = head;
    uint8_t cmp_sz = std::min((int)strlen(fname), FNAME_MAX - 1);

    while (NULL != search) {
        if (strncmp(fname, search->fname, cmp_sz) == 0) {
            return search;
        }
        search = search->next;
    }

    return NULL;
}

uint8_t thumbnail_prepare(std::filesystem::path file)
{
    node_t *node_ptr;
    node_t *node_s;

    if ((file.extension().string().compare(".dtv") == 0) || 
        (file.extension().string().compare(".jpg") == 0)) {
        if (head == NULL) {
            node_ptr = ll_add(&head);
            strncpy(node_ptr->fname, file.filename().c_str(), FNAME_MAX - 1);
            node_ptr->flags = FL_FILE_PREPARE;
            //node_populate(node_ptr, file.c_str());
        } else {
            if ((node_s = node_search_fname(file.filename().c_str())) == NULL) {
                node_ptr = ll_add(&head);
                strncpy(node_ptr->fname, file.filename().c_str(), FNAME_MAX - 1);
                node_ptr->flags = FL_FILE_PREPARE;
                //node_populate(node_ptr, file.c_str());
            //} else {
            //    if (node_s->flags == FL_FILE_PREPARE) {
            //        node_populate(node_s, file.c_str());
            //    }
            }
        }

        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

void file_library(bool *p_open, th_db_t * db)
{
    if (!ImGui::Begin("image library", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }
    uint32_t texture = 0;
    static float padding = 32.0f;
    float thumbnail_size_x = thumbnail_size;
    float thumbnail_size_y = thumbnail_size;
    float cell_size = thumbnail_size_x + padding;
    std::filesystem::path abs_path;
    uint16_t path_size = 0;
    uint8_t worthy_file = 0;
    //time_t current_time;
    //static time_t last_discovery = 0;
    node_t *search = NULL;
    uint32_t u;
    uint8_t entry_is_dir = 0;

    float panel_width = ImGui::GetContentRegionAvail().x;
    int column_count = (int)(panel_width / cell_size);

    if (column_count < 1) {
        column_count = 1;
    }

    ImGui::Columns(column_count, 0, false);
    static std::filesystem::path m_current_directory = std::filesystem::current_path();

    ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);     // Black background
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint

    ImGui::PushID("../");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::ImageButton("", (void *)(intptr_t) dir_tx, {
                       thumbnail_size_x, thumbnail_size_y}, {
                       0, 0}, {
                       1, 1}, bg_col, tint_col);
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        m_current_directory = m_current_directory.parent_path();
    }
    ImGui::Text("../");
    ImGui::NextColumn();
    ImGui::PopID();

#if 0
    current_time = time(NULL);

    if ((current_time - last_discovery) > 1) {
        //file_library_discovery(m_current_directory);
        last_discovery = current_time;
    }
#endif

    // add directories directly into the library
    // files are added only after they are deemed ready by file_library_discovery()
    // by that time their texture thumbnail has been generated
 for (auto & directory_entry:std::filesystem::directory_iterator(m_current_directory)) {
        const auto & path = directory_entry.path();
        std::string filename_string = path.filename().string();
        std::string filename_ext = path.extension().string();

        entry_is_dir = 0;
        worthy_file = 0;
        thumbnail_size_y = thumbnail_size;

        if (directory_entry.is_directory()) {
            texture = dir_tx;
            entry_is_dir = 1;
        } else {
            texture = file_tx;
            abs_path = m_current_directory;
            abs_path /= filename_string;

            if ((search = node_search_fname(filename_string.c_str())) != NULL) {
                // if file has been already opened and a texture has been created for it's thumbnail
                if (search->flags & FL_FILE_READY) {
                    worthy_file = 1;
                    if (search->texture) {
                        texture = search->texture;
                        u = search->height * thumbnail_size_x / search->width;
                        thumbnail_size_y = u;
                    }
                } else if (search->flags & FL_FILE_PREPARE) {
                    worthy_file = 1;
                    node_populate(search, abs_path.c_str());
                }
            } else {
                // file not present in the linked list
                if (thumbnail_prepare(abs_path) == EXIT_SUCCESS) {
                    worthy_file = 1;
                }
            }
            if (!worthy_file) {
                continue;
            }
        }

        ImGui::PushID(filename_string.c_str());
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton("", (void *)(intptr_t) texture, {
                           thumbnail_size_x, thumbnail_size_y}, {
                           0, 0}, {
                           1, 1}, bg_col, tint_col);

        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (entry_is_dir) {
                m_current_directory /= path.filename();
            } else {
                if (search && (search->flags & FL_FILE_READY)) {
                    cleanup(db);
                    if (db->p.in_file) {
                        free(db->p.in_file);
                    }

                    path_size = strlen(abs_path.c_str());
                    db->p.in_file = (char *)calloc(path_size + 1, sizeof(char));
                    memcpy(db->p.in_file, abs_path.c_str(), path_size + 1);
                    db->p.in_file[path_size] = 0;
                    db->fe.return_state = RET_RST;
                    main_cli(db, 0);
                    viewport_refresh_vp(db);
                } else {
                    fprintf(stderr, "warning: unable to open %s\n", abs_path.c_str());
                }
            }
        }

        ImGui::TextWrapped(filename_string.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::End();
}
