// Added by LucaPancallo 2000.09.28
void OutpHeadCS(FILE * o, H3dsScene * scene, int verts, char * name);
void OutpVertsCS(FILE * o, H3dsMapVert * vrtmap, int verts,char * name);
void OutpCentresCS(FILE * o, H3dsScene * scene, char * name);
void OutpObjectsCS(FILE * o, H3dsScene * scene, H3dsMapVert* vrtmap, int verts, char * name, bool lighting);
