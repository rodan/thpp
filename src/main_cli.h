
#ifndef __MAIN_CLI_H__
#define __MAIN_CLI_H__

#ifdef __cplusplus
extern "C" {
#endif

int main_cli(th_db_t *db, uint8_t flags);
void cleanup(th_db_t *db);

#ifdef __cplusplus
}
#endif

#endif
