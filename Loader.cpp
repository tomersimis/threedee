#include "Loader.h"
#include <string>

//#define READ_TEXTURE
//#define DBG
//#define DBG_PARSE_FACE
#define IGNORE_NO_TYPE

/*TYPES*/
#define NO_TYPE 0
#define VERTICE 1 
#define NORMAL 2 
#define FACE 3 
#define TEXTURE 4 

/*ERRORS*/
#define NO_ERROR 0 
#define NO_FILE_FOUND 1
#define NO_MORE_2_VALUES 2
#define NO_TYPE_FOUND 3
#define LENGTH_ERROR  4

Loader::Loader(string *_path, vector<Point> *_points, vector<Normal> *_normals, vector<Face> *_faces){
	path = _path;
	points = _points;
	normals = _normals;
	faces = _faces;
}

Loader::Loader(){
}

/*
	i and j inclusive
*/

double Loader::parseDouble(int i, int j) {
	double res =0, resDecimal=0;
	int pt = 0, signal =1, doc= j+1;
	if(strParse[i]=='-'){
		signal = -1; i++;
	}
	double x;
	for(int k =i; k<=j; k++) if(strParse[k] == '.' || strParse[k]== ',' ) { doc = k; break; }
	for(int k = doc -1; k>=i ; k--, pt++){ // summing all integer values
		res +=  ((strParse[k]-'0')+0.0)*pow(10.0, pt);
	}
	pt = -1;
	for(int k = doc + 1; k <= j; k++, pt--){ // summing all decimal values ( if they exist )
		res += ((strParse[k]-'0') +0.0)*pow(10.0, pt);
	}

	return res*signal;
}


typedef long long int lld;

long long int Loader::parseInt(int i, int j){
	int signal =1, pt=0; long long int res =0 ;
	if(strParse[i]=='-' ) {
		signal = -1; i++;
	}
	for(int k=j; k>= i; k--, pt++){
		res += ( lld( strParse[k]-'0' ) )*(lld(pow(10.0, pt)));
	}
	return res*signal;
}


int Loader::findPatternFace(int len) {
	int next;
	bool found= 0;
	for(int i=1; i<len; i++){
		if(strParse[i] != '/' ) continue;
		found =1;
		next = i+1;
		if(next < len) {
			if( strParse[next] == '/' ){ // 2 slashs recognize type 1 and 2
				next++;
				if ( next < len && strParse[next] != 32 ) {
					return 2;
				}
				else return 1;
				
			}
		}
	}
	if(found) return 3;
	return 4;
}

Face Loader::parseFace(int pattern){
/*
	(f) faces: in formats: 
	type 1-> 	1// (only points to a vertex)
	type 2-> 1//1 (links a vertex to a normal)
	type 3-> 1/1/1 (has all three)
	type 4-> 1 1 1	
*/	


	Face face;
	Point pa, pb, pc; Normal na, nb, nc	;
	Point paux; Normal naux;
	int len = strParse.length();
	#ifdef  DBG_PARSE_FACE
	printf("parseFace call -> %s Pattern:%d \n", strParse.c_str(), pattern);
	#endif
	int cont =0;
	bool norm = 0;
	long long int num;	
	if( pattern == 1 || pattern == 4 || pattern == 2 ){
		for(int i=1; i < len; i++){
			if (strParse[i] >= '0' && strParse[i] <= '9' ) {
				for(int j = i; j< len; j++){
					if( j+1 >=len || strParse[j+1] == 32 || strParse[j+1] == '/') {
						num = parseInt(i,j);
						num--; // INDEX by >=1
						if(num < points->size() ) paux = points->at(num);			
						if(num < normals->size() ) naux = normals->at(num);		
						#ifdef  DBG_PARSE_FACE
						printf("NUM >%lld  |  %lf %lf %lf\n", num , paux.x , paux.y, paux.z);	
						#endif 
						if(cont==0){	
//							printf("ponto -> %lf %lf %lf\n", pa.x, pa.y , pa.z);
							if(! (pattern==2) || ( (pattern==2) && !norm) ) {
								#ifdef  DBG_PARSE_FACE
								printf("entrou em num-> %lld\n", num);
								#endif								
								pa.x = paux.x;
								pa.y = paux.y;
								pa.z = paux.z;													 
								if(! (pattern==2)) cont++;
								norm =1 ;
							}
							else if ( (pattern==2) && norm){
								na.x = naux.x; na.y = naux.y; na.z = naux.z;
								cont++;
								norm = 0;
							}						
						 }
						else if(cont == 1) {
							if(! (pattern==2) || ( (pattern==2) && !norm) ) {
								pb.x = paux.x;
								pb.y = paux.y;
								pb.z = paux.z;
								if(! (pattern==2)) cont++;								
								norm =1;
							}
							else if ( (pattern==2) && norm){
								nb.x = naux.x; nb.y = naux.y; nb.z = naux.z;
								cont++;
								norm = 0;								
							}
							
						 }
						else if (cont == 2) {
							if(! (pattern==2) || ( (pattern==2) && !norm) ) {						 
								pc.x = paux.x;
								pc.y = paux.y;
								pc.z = paux.z;
								if(! (pattern==2)) cont++;
								norm = 1;
							}
							else if ((pattern==2) && norm){
								nc.x = naux.x; nc.y = naux.y; nc.z = naux.z;
								cont++;
								norm = 0;								
							}							
						}
						i=j;
						break;
					}
				}
			}	
		}
		
		if(pattern == 1 || pattern == 4) {
			#ifdef  DBG_PARSE_FACE	
			printf("adding face -> %lf %lf %lf ||  %lf %lf %lf ||  %lf %lf %lf \n", face.v1->x, face.v1->y, face.v1->z,  face.v2->x, face.v2->y, face.v2->z  ,  face.v3->x, face.v3->y, face.v3->z    );		
			#endif
			faces->push_back( Face(pa, pb, pc)   );
		}
		if(pattern == 2 ) {
			#ifdef  DBG_PARSE_FACE		
			printf("adding face WITH NORMAL-> %lf %lf %lf ||  %lf %lf %lf ||  %lf %lf %lf \n", face.v1->x, face.v1->y, face.v1->z,  face.v2->x, face.v2->y, face.v2->z  ,  face.v3->x, face.v3->y, face.v3->z    );		
			printf("NORMAL -> %lf %lf %lf | %lf %lf %lf | %lf %lf %lf\n", na.x, na.y, na.z, nb.x, nb.y, nb.z, nc.x, nc.y, nc.z);
			#endif
			faces->push_back( Face(pa, pb, pc, na, nb, nc)   );		
			#ifdef  DBG_PARSE_FACE			
			face = faces->at(  int(faces->size())  - 1 );
			printf("NORMAL ADDED-> %lf %lf %lf | %lf %lf %lf | %lf %lf %lf\n", na.x, na.y, na.z, nb.x, nb.y, nb.z, nc.x, nc.y, nc.z);	
			#endif		
		}
	}

	int ms = faces->size();
	face = faces->at(0);
	#ifdef  DBG_PARSE_FACE	
	printf("FIM -> %lf %lf %lf ||  %lf %lf %lf ||  %lf %lf %lf \n", face.v1->x, face.v1->y, face.v1->z,  face.v2->x, face.v2->y, face.v2->z  ,  face.v3->x, face.v3->y, face.v3->z    );	
	#endif 
}

int Loader::load() {
	string x; int l, t;	
	string line;
	int lineCount=0, len;
	long long int arrInt[10], laux;
	double arrDouble[5], daux;
	int cont;
	Point point;
	int ERROR=NO_ERROR;
	
	ifstream file ( (*(path)).c_str() );
	if (file.is_open()) {
		while ( 1  ) {

			if( ! ( getline (file,line) )) break;		
			lineCount++;						
			#ifdef DBG
			cout << "Linha["<<lineCount<<"]: " << line << endl;
			#endif
			if(line[0]=='#' || line[0] == '/') {
				#ifdef DBG	
				cout << "** Comentario detected, first character: " << line[0] << endl;
				#endif			
				continue;
			}
			len = line.length();
			if( !len ) continue;
			if( len < 2 ) {
				ERROR = LENGTH_ERROR;
				goto LABEL_ERROR;
			}
			/*
				TYPE:
				0- > doesn't exist
				1 -> normal
				2 -> normal
				3 -> face
			*/
			int TYPE=NO_TYPE;
			if(line[0]=='v' && line[1]=='n') TYPE = NORMAL;
			else if (line[0]=='v' && line[1]=='t' ) TYPE = TEXTURE;			
			else if(line[0]=='v') TYPE = VERTICE ;
			else if (line[0]=='f') TYPE = FACE; 

			strParse = line; 
			
			if(TYPE == NO_TYPE ) {
				#ifdef IGNORE_NO_TYPE
					continue;
				#endif 	
				
				ERROR = NO_TYPE_FOUND; 
				goto LABEL_ERROR;
		
			}			
			if( TYPE == VERTICE || TYPE == NORMAL ) {
				cont = 0;	
				
				#ifdef DBG
					printf("tyoe: %d len %d\n", TYPE , len);
				#endif 				
														
				for(int i= (VERTICE?2:1) ; i< len; i++){
					if(line[i]!=32) {//whitespace
				#ifdef DBG
					printf("found: %c %d\n", line[i], i );
				#endif 		
						for(int j=i; j<len; j++){
						
							if(line[j]==32) {
				#ifdef DBG
					printf("stop at: %d\n", j );
				#endif 			
								daux = parseDouble(i,j-1);
								arrDouble[cont] = daux; cont++;
								i = j-1; break;
							}
							else if (j==len-1 && line[j] != 32 ) { // last number and not a white space
								daux = parseDouble(i,j);
								arrDouble[cont] = daux; cont++;
								i= j; break;
							}
						}
					}				
				}
				
				#ifdef DBG
					printf("final cont:%d\n", cont);
					for(int i=0; i < cont; i++){
						printf("%lf ", arrDouble[i]);
					}
					if(cont) printf("\n");
				#endif
				
				if(cont==3){
					point = Point(arrDouble[0], arrDouble[1], arrDouble[2]);
				}
				else if(cont==2) {
					point = Point(arrDouble[0], arrDouble[1]);
				}
				else {
					ERROR = NO_MORE_2_VALUES; 
					goto LABEL_ERROR;
				}
				if(TYPE == VERTICE){
					#ifdef DBG
						printf("Adding point %lf %lf %lf\n", point.x, point.y, point.z);
					#endif 
					points->push_back(point); 
				}
				if ( TYPE == NORMAL ) {
					Normal normal = Normal(point.x, point.y);
					if( cont  == 3) normal.z = point.z;
					#ifdef DBG
						printf("Adding normal %lf %lf\n", normal.x, normal.y ) ;
					#endif 					
					normals->push_back(normal); 
				}
			}
			else if (TYPE == FACE) {
				#ifdef DBG
				printf("ENTROU FACE FIND : %s\n", strParse.c_str());
				#endif 
				int pattern = findPatternFace(len);
				#ifdef DBG
				printf("ENCONTROU APTTEND %d : %s\n", pattern, strParse.c_str());				
				#endif
				parseFace(pattern);

			}
			/*other types*/	
		}
	}
	else {
		ERROR = NO_FILE_FOUND;
		goto LABEL_ERROR;
	}    
	LABEL_ERROR:;
	#ifdef DBG	
		int s= points->size();
		printf("Read -> %d \n", s);
	#endif 	
	/*
		ERROR =0 --> OK	
		ERROR =1 --> FILE NOT FOUND
		ERROR =2 --> VERTICE AND NORMALS DOESN'T MATCH 2 OR 3 VALUES;
		ERROR =3 --> TYPE DOESN'T RECOGNIZED
		error 4 = LENGTH_ERROR
		
	*/
	if(ERROR) {
		cout << "ERROR [TYPE:" << ERROR << "]" << endl;
	}
	else {
//		#ifdef DBG
			cout << "File read with sucess!" << endl;
			cout << "Points: " << int(points->size()) << endl;
			cout << "Normals: " << int(normals->size()) << endl;
			cout << "Faces: " << int(faces->size()) << endl;						
	//	#endif 
	}
	
	file.close();      
}

/*
g++ teste.cpp  Point.h Point.cpp Vector.cpp Normal.h Normal.cpp Face.h Face.cpp Loader.h Loader.cpp

*/

/*
void testParser(){
	double x; int l;
	strParse = "-23.3"; l = strParse.length() -1;
	x = parseDouble(0,l);	
	printf("x -> %lf\n", x); 
	strParse = "-1"; l = strParse.length() -1;
	x = parseDouble(0,l);	
	printf("x -> %lf\n", x);
	strParse = "0"; l = strParse.length() -1;
	x = parseDouble(0,l);	
	printf("x -> %lf\n", x);
	strParse = "3.33333"; l = strParse.length() -1;
	x = parseDouble(0,l);	
	printf("x -> %lf\n", x);		
	strParse = "-3.33333"; l = strParse.length() -1;
	x = parseDouble(0,l);	
	printf("x -> %lf\n", x);	
	strParse = "25.0"; l = strParse.length() -1;
	x = parseDouble(0,l);	
	printf("x -> %lf\n", x);		
	strParse = "-0"; l = strParse.length() -1;
	x = parseDouble(0,l);	
	printf("x -> %lf\n", x);		
	strParse = "-12"; l = strParse.length() -1;
	x = parseDouble(0,l);	
	printf("x -> %lf\n", x);	
	
}

test PATTERN :

	strParse = "3/3/3 2/2/2 112121/1121251251/111"; l = strParse.length();
	t = findPatternFace(l);
	printf("pat -> %d\n", t);
	strParse = "123//32 22//232 23//23"; l = strParse.length();
	t = findPatternFace(l);
	printf("pat -> %d\n", t);	
	strParse = "3 3 3"; l = strParse.length();
	t = findPatternFace(l);
	printf("pat -> %d\n", t);
	strParse = "999999999999999999999999/999999999999999999/9999999999"; l = strParse.length();
	t = findPatternFace(l);
	printf("pat -> %d\n", t);
	
*/


/*
	(f) faces: in formats: 
	type 1-> 	1// (only points to a vertex)
	type 2-> 1//1 (links a vertex to a normal)
	type 3-> 1/1/1 (has all three)
	type 4-> 1 1 1	
*/





