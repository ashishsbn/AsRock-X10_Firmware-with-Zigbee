#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 

char end_point[50];
char end_point1[50];

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   strcpy(end_point,"hello") ;
   strcpy(end_point,argv[i] ? argv[i] : "NULL");
   fprintf(stderr, "%s:", (const char*)data);
   
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char* argv[]) {
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char *data;

   /* Open database */
   rc = sqlite3_open("/etc/orbweb.db", &db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }

   /* Create merged SQL statement */
   sql = "select NAME from DEVICES where IEEEAddress='00124B0008B15AC7'; "; 
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   
   if( rc != SQLITE_OK ) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Operation done successfully\n");
   }
   sqlite3_close(db);
   return 0;
}
