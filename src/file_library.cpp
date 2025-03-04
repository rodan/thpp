
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <map>
#include <omp.h>
#include <unistd.h>
#include "tlpi_hdr.h"
#include "imgui.h"
#include "proj.h"
#include "opengl_helper.h"
#include "viewport.h"
#include "main_cli.h"
#include "version.h"
#include "file_library.h"

#define           CONFIG_OMP
#define   CONFIG_OMP_THREADS  12
#define            FNAME_MAX  1024
//possible flags
#define        FL_FILE_READY  0x1
#define      FL_FILE_INVALID  0x2
#define      FL_FILE_PREPARE  0x4
#define     FL_FILE_NEED_TEX  0x8

#define  FL_REFRESH_INTERVAL  10  //// < number of seconds between consecutive filesystem refreshes

using namespace std;

#ifndef __has_include
  static_assert(false, "__has_include not supported");
#else
#  if __cplusplus >= 201703L && __has_include(<filesystem>)
#    include <filesystem>
     namespace fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
     namespace fs = std::experimental::filesystem;
#  elif __has_include(<boost/filesystem.hpp>)
#    include <boost/filesystem.hpp>
     namespace fs = boost::filesystem;
#  endif
#endif

uint32_t file_tx;
uint32_t dir_tx;

struct node {
    uint16_t flags;
    char fname[FNAME_MAX];
    struct stat st;
    uint32_t texture;
    uint16_t width;
    uint16_t height;
    th_db_t *thumb;
    struct node *next;
};
typedef struct node node_t;

node_t *head = NULL;

void ll_print(node_t * head);
node_t *ll_find_tail(node_t * head);
node_t *ll_add(node_t ** head, const uint8_t val);
node_t *ll_remove(node_t ** head, const uint8_t val);
uint32_t ll_count(node_t * head);
node_t *ll_get_by_index(node_t * head, uint32_t search_idx);

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
        //del->thumb is freed in file_library()
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
        errMsg("malloc error");
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

uint32_t ll_count(node_t * head)
{
    uint32_t ret = 0;

    node_t *p = head;

    while (NULL != p) {
        p = p->next;
        ret++;
    }

    return ret;
}

node_t *ll_get_by_index(node_t * head, uint32_t search_idx)
{
    uint32_t index = 0;

    node_t *p = head;

    while (NULL != p) {
        if (search_idx == index) {
            return p;
        }
        p = p->next;
        index++;
    }

    return NULL;
}

void file_library_init(void)
{
    unsigned int w, h;
    file_tx = 0;
    dir_tx = 0;

#if defined(__linux__)
    if (load_texture_from_file("res/file_icon.png", &file_tx, &w, &h) != EXIT_SUCCESS) {
        if (load_texture_from_file("/usr/share/thpp/file_icon.png", &file_tx, &w, &h) != EXIT_SUCCESS) {
            fprintf(stderr, "error loading file icon\n");
        }
    }
    if (load_texture_from_file("res/dir_icon.png", &dir_tx, &w, &h) != EXIT_SUCCESS) {
        if (load_texture_from_file("/usr/share/thpp/dir_icon.png", &dir_tx, &w, &h) != EXIT_SUCCESS) {
            fprintf(stderr, "error loading directory icon\n");
        }
    }
#elif defined(__FreeBSD__)
    if (load_texture_from_file("res/file_icon.png", &file_tx, &w, &h) != EXIT_SUCCESS) {
        if (load_texture_from_file("/usr/local/share/thpp/file_icon.png", &file_tx, &w, &h) != EXIT_SUCCESS) {
            fprintf(stderr, "error loading file icon\n");
        }
    }
    if (load_texture_from_file("res/dir_icon.png", &dir_tx, &w, &h) != EXIT_SUCCESS) {
        if (load_texture_from_file("/usr/local/share/thpp/dir_icon.png", &dir_tx, &w, &h) != EXIT_SUCCESS) {
            fprintf(stderr, "error loading directory icon\n");
        }
    }
#endif
}

void file_library_free(void)
{
    ll_free_all(&head);
}

std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),[](unsigned char c) { return std::tolower(c);} // correct
    );
    return s;
}

uint8_t node_populate(node_t * node)
{
    uint8_t ret = EXIT_FAILURE;
    global_preferences_t *pref = gp_get_ptr();
    size_t fname_sz;

    //printf("node_populate %p %d %s\n", (void *) node, node->flags, node->fname);
    if (stat(node->fname, &node->st) < 0) {
        node->flags = FL_FILE_INVALID;
        errMsg("stat() error for %s", node->fname);
        goto cleanup;
    }

    node->thumb = (th_db_t *) calloc(1, sizeof(th_db_t));
    if (node->thumb == NULL) {
        errMsg("calloc() error");
        goto cleanup;
    }

    fname_sz = strlen(node->fname);
    node->thumb->p.in_file = (char *)calloc(fname_sz + 1, sizeof(char));
    if (node->thumb->p.in_file == NULL) {
        errMsg("calloc() error");
        goto cleanup;
    }

    strcpy(node->thumb->p.in_file, node->fname);
    node->thumb->p.pal = pref->palette_default;
    node->thumb->p.zoom_level = 1;

    if (main_cli(node->thumb, 0) == EXIT_SUCCESS) {
        node->width = node->thumb->rgba[0].width;
        node->height = node->thumb->rgba[0].height;
        node->flags = FL_FILE_NEED_TEX;
        ret = EXIT_SUCCESS;
    } else {
        fprintf(stderr, "warning: %s can't be opened as a thermal image\n", node->fname);
        node->flags = FL_FILE_INVALID;
    }

cleanup:

    return ret;
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

uint8_t thumbnail_prepare(fs::path file)
{
    node_t *node_ptr;
    node_t *node_s;

    if ((file.extension().string().compare(".dtv") == 0) ||
        (file.extension().string().compare(".DTV") == 0) ||
        (file.extension().string().compare(".jpg") == 0)) {
        if (head == NULL) {
            node_ptr = ll_add(&head);
            //strncpy(&node_ptr->fname[0], file.filename().c_str(), FNAME_MAX - 1);
            strncpy(&node_ptr->fname[0], file.c_str(), FNAME_MAX - 1);
            node_ptr->flags = FL_FILE_PREPARE;
        } else {
            //node_s = node_search_fname(file.filename().c_str());
            node_s = node_search_fname(file.c_str());
            if (node_s == NULL) {
                node_ptr = ll_add(&head);
                //strncpy(&node_ptr->fname[0], file.filename().c_str(), FNAME_MAX - 1);
                strncpy(&node_ptr->fname[0], file.c_str(), FNAME_MAX - 1);
                node_ptr->flags = FL_FILE_PREPARE;
            }
        }

        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

template <typename TP>
time_t to_time_t(TP tp) {
    using namespace chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}

void file_library(bool *p_open, th_db_t * db)
{
    uint32_t texture = 0;
    static float padding = 32.0f;
    global_preferences_t *pref = gp_get_ptr();
    float thumbnail_size_x = pref->thumbnail_size;
    float thumbnail_size_y = pref->thumbnail_size;
    float cell_size = thumbnail_size_x + padding;
    fs::path abs_path;
    uint16_t path_size = 0;
    uint8_t worthy_file = 0;
    time_t current_time;
    static time_t last_discovery = 0;
    node_t *search = NULL;
    uint32_t u;
    uint8_t entry_is_dir = 0;
    double runtime;
    node_t *node;
    static map<fs::directory_entry, time_t> sorted_entries;

    if (!ImGui::Begin("image library", p_open, 0)) {
        ImGui::End();
        return;
    }

    float panel_width = ImGui::GetContentRegionAvail().x;
    int column_count = (int)(panel_width / cell_size);

    if (column_count < 1) {
        column_count = 1;
    }

    ImGui::Columns(column_count, 0, false);
    static fs::path m_current_directory = fs::current_path();

    ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);     // Black background
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint

    ImGui::PushID("../");
//    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::ImageButton("", (ImTextureID)(intptr_t) dir_tx, {
                       thumbnail_size_x, thumbnail_size_y}, {
                       0, 0}, {
                       1, 1}, bg_col, tint_col);
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        m_current_directory = m_current_directory.parent_path();
        last_discovery = 0;
    }
    ImGui::Text("../");
    ImGui::NextColumn();
    ImGui::PopID();

    current_time = time(NULL);

    if ((current_time - last_discovery) > FL_REFRESH_INTERVAL) {
        // cleanup map
        for (auto it = sorted_entries.begin(); it != sorted_entries.end(); ) {
            sorted_entries.erase(it++);
        }

        // repopulate map
        for (auto &entry:fs::directory_iterator(m_current_directory)) {
            fs::file_status s = entry.symlink_status();
            if (fs::is_regular_file(s) || fs::is_directory(s)) {
                auto time = to_time_t(entry.last_write_time());
                sorted_entries[entry] = time;
            }
        }

        //file_library_discovery(m_current_directory);
        last_discovery = current_time;
    }

    // add directories directly into the library
    // files are added only after they are deemed ready by file_library_discovery()
    // by that time their texture thumbnail has been generated
    //for (auto & entry:fs::directory_iterator(m_current_directory)) {
    for (auto const &[entry, time] : sorted_entries) {

        const auto & path = entry.path();
        std::string filename_string = path.filename().string();
        std::string filename_ext = path.extension().string();

        entry_is_dir = 0;
        worthy_file = 0;
        thumbnail_size_y = pref->thumbnail_size;

        if (entry.is_directory()) {
            texture = dir_tx;
            entry_is_dir = 1;
        } else {
            texture = file_tx;
            abs_path = m_current_directory;
            abs_path /= filename_string;

            //if ((search = node_search_fname(filename_string.c_str())) != NULL) {
            if ((search = node_search_fname(abs_path.c_str())) != NULL) {
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
#if 0
                    if (file_analyze) {
                        node_populate(search);
                        file_analyze--;
                    }
#endif
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
//        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton("", (ImTextureID)(intptr_t) texture, {
                           thumbnail_size_x, thumbnail_size_y}, {
                           0, 0}, {
                           1, 1}, bg_col, tint_col);
//        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (entry_is_dir) {
                m_current_directory /= path.filename();
                last_discovery = 0; // force cleanup of the sorted_entries map
                ImGui::PopID();
                break;
            } else {
                // an infrared image file is double-clicked
                if (search && (search->flags & FL_FILE_READY)) {
                    cleanup(db);
                    if (db->p.in_file) {
                        free(db->p.in_file);
                    }

                    path_size = strlen(abs_path.c_str());
                    db->p.in_file = (char *)calloc(path_size + 1, sizeof(char));
                    if (db->p.in_file != NULL) {
                        memcpy(db->p.in_file, abs_path.c_str(), path_size + 1);
                        db->p.in_file[path_size] = 0;
                        db->fe.return_state = RET_RST;
                        //db->p.zoom_level = 1;
                        main_cli(db, 0);
                        if (db->fe.flags & HIGHLIGHT_LAYER_EN) {
                            //generate_highlight(db);
                            refresh_highlight_vp(db);
                        }
                        viewport_refresh_vp(db);
                    } else {
                        errMsg("calloc error");
                    }
                } else {
                    fprintf(stderr, "warning: unable to open %s\n", abs_path.c_str());
                }
            }
        }

        ImGui::TextWrapped("%s", filename_string.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }

    runtime = omp_get_wtime();

#if defined CONFIG_OMP
    uint32_t thumb_list_sz = ll_count(head);
    omp_set_num_threads(CONFIG_OMP_THREADS);

#pragma omp parallel private(node)
    {
        //node_t *node;
        uint32_t i;

        int t_cnt = omp_get_num_threads();
        int t_cur = omp_get_thread_num();

        for (i = t_cur; i < thumb_list_sz; i += t_cnt) {
            node = ll_get_by_index(head, i);

            if (node == NULL) {
                continue;
            }

            if (node->flags & FL_FILE_PREPARE) {
                node_populate(node);
            }
        }
    }

#else
    // single thread
    //node_t *node;
    node = head;

    // single thread
    while (node != NULL) {
        if (node->flags & FL_FILE_PREPARE) {
            node_populate(node);
        }
        node = node->next;
    }
#endif

    runtime = omp_get_wtime() - runtime;
    if (runtime > 0.01) {
        printf("album ready in %lfs\n", runtime);
    }

    node = head;
    while (node != NULL) {
        if (node->flags & FL_FILE_NEED_TEX) {
            if (node->thumb != NULL) {

                load_texture_from_mem(node->thumb->rgba[0].data, &node->texture, node->thumb->rgba[0].width,
                              node->thumb->rgba[0].height);

                //printf("tex %u sz %dx%d for %s, node %p\n", node->texture, node->thumb->rgba[0].width,
                //              node->thumb->rgba[0].height, node->fname, (void *) node);

                if (node->texture) {
                    node->flags = FL_FILE_READY;
                    if (node->thumb->p.in_file) {
                        free(node->thumb->p.in_file);
                    }
                    cleanup(node->thumb);
                    free(node->thumb);
                    node->thumb = NULL;
                } else {
                    fprintf(stderr, "warning, empty texture\n");
                }
            } else {
                fprintf(stderr, "error, node->thumb already freed\n");
            }
        }
        node = node->next;
    }

    ImGui::End();
}
