#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const char* colors[] = {"FFFFFF","FEFEFE","FDFDFD","FCFCFC","FBFBFB","FAFAFA","F9F9F9","F8F8F8","F7F7F7","F6F6F6","F5F5F5","F4F4F4","F3F3F3","F2F2F2","F1F1F1","F0F0F0","EFEFEF","EEEEEE","EDEDED","ECECEC","EBEBEB","EAEAEA","E9E9E9","E8E8E8","E7E7E7","E6E6E6","E5E5E5","E4E4E4","E3E3E3","E2E2E2","E1E1E1","E0E0E0","DFDFDF","DEDEDE","DDDDDD","DCDCDC","DBDBDB","DADADA","D9D9D9","D8D8D8","D7D7D7","D6D6D6","D5D5D5","D4D4D4","D3D3D3","D2D2D2","D1D1D1","D0D0D0","CFCFCF","CECECE","CDCDCD","CCCCCC","CBCBCB","CACACA","C9C9C9","C8C8C8","C7C7C7","C6C6C6","C5C5C5","C4C4C4","C3C3C3","C2C2C2","C1C1C1","C0C0C0","BFBFBF","BEBEBE","BDBDBD","BCBCBC","BBBBBB","BABABA","B9B9B9","B8B8B8","B7B7B7","B6B6B6","B5B5B5","B4B4B4","B3B3B3","B2B2B2","B1B1B1","B0B0B0","AFAFAF","AEAEAE","ADADAD","ACACAC","ABABAB","AAAAAA","A9A9A9","A8A8A8","A7A7A7","A6A6A6","A5A5A5","A4A4A4","A3A3A3","A2A2A2","A1A1A1","A0A0A0","9F9F9F","9E9E9E","9D9D9D","9C9C9C","9B9B9B","9A9A9A","999999","989898","979797","969696","959595","949494","939393","929292","919191","909090","8F8F8F","8E8E8E","8D8D8D","8C8C8C","8B8B8B","8A8A8A","898989","888888","878787","868686","858585","848484","838383","828282","818181","808080","7F7F7F","7E7E7E","7D7D7D","7C7C7C","7B7B7B","7A7A7A","797979","787878","777777","767676","757575","747474","737373","727272","717171","707070","6F6F6F","6E6E6E","6D6D6D","6C6C6C","6B6B6B","6A6A6A","696969","686868","676767","666666","656565","646464","636363","626262","616161","606060","5F5F5F","5E5E5E","5D5D5D","5C5C5C","5B5B5B","5A5A5A","595959","585858","575757","565656","555555","545454","535353","525252","515151","505050","4F4F4F","4E4E4E","4D4D4D","4C4C4C","4B4B4B","4A4A4A","494949","484848","474747","464646","454545","444444","434343","424242","414141","404040","3F3F3F","3E3E3E","3D3D3D","3C3C3C","3B3B3B","3A3A3A","393939","383838","373737","363636","353535","343434","333333","323232","313131","303030","2F2F2F","2E2E2E","2D2D2D","2C2C2C","2B2B2B","2A2A2A","292929","282828","272727","262626","252525","242424","232323","222222","212121","202020","1F1F1F","1E1E1E","1D1D1D","1C1C1C","1B1B1B","1A1A1A","191919","181818","171717","161616","151515","141414","131313","121212","111111","101010","0F0F0F","0E0E0E","0D0D0D","0C0C0C","0B0B0B","0A0A0A","090909","080808","070707","060606","050505","040404","030303","020202","010101","000000"};
const char* chars[] = {"aaa","aab","aac","aad","aae","aaf","aag","aah","aai","aaj","aak","aal","aam","aan","aao","aap","aaq","aar","aas","aat","aau","aav","aaw","aax","aay","aaz","aba","abb","abc","abd","abe","abf","abg","abh","abi","abj","abk","abl","abm","abn","abo","abp","abq","abr","abs","abt","abu","abv","abw","abx","aby","abz","aca","acb","acc","acd","ace","acf","acg","ach","aci","acj","ack","acl","acm","acn","aco","acp","acq","acr","acs","act","acu","acv","acw","acx","acy","acz","ada","adb","adc","add","ade","adf","adg","adh","adi","adj","adk","adl","adm","adn","ado","adp","adq","adr","ads","adt","adu","adv","adw","adx","ady","adz","aea","aeb","aec","aed","aee","aef","aeg","aeh","aei","aej","aek","ael","aem","aen","aeo","aep","aeq","aer","aes","aet","aeu","aev","aew","aex","aey","aez","afa","afb","afc","afd","afe","aff","afg","afh","afi","afj","afk","afl","afm","afn","afo","afp","afq","afr","afs","aft","afu","afv","afw","afx","afy","afz","aga","agb","agc","agd","age","agf","agg","agh","agi","agj","agk","agl","agm","agn","ago","agp","agq","agr","ags","agt","agu","agv","agw","agx","agy","agz","aha","ahb","ahc","ahd","ahe","ahf","ahg","ahh","ahi","ahj","ahk","ahl","ahm","ahn","aho","ahp","ahq","ahr","ahs","aht","ahu","ahv","ahw","ahx","ahy","ahz","aia","aib","aic","aid","aie","aif","aig","aih","aii","aij","aik","ail","aim","ain","aio","aip","aiq","air","ais","ait","aiu","aiv","aiw","aix","aiy","aiz","aja","ajb","ajc","ajd","aje","ajf","ajg","ajh","aji","ajj","ajk","ajl","ajm","ajn","ajo","ajp","ajq","ajr","ajs","ajt","aju","ajv"};

struct idxNumbs
{
	int idx;
	int corr;
	int guess;
};

int main(int argc, char* argv[])
{
	system("rm -R WrongOnes/");
	system("mkdir WrongOnes");
	//Prima fase: leggo idxNumb
	int numIdx;
	int idxMax;
	int currI=0;
	int currSearch;
	idxNumbs* numbers; 
	FILE* idx = fopen("numIdxErr", "r");
	if (idx == nullptr) return 1;
	fscanf(idx, "%d\n", &numIdx);
	printf("numIdx: %d\n", numIdx);
	fclose(idx);
	idx = fopen("idxNumb", "r");
	if (idx == nullptr) return 1;
	numbers = new idxNumbs[numIdx];
	
	for (int i=0; i<numIdx; i++)
		fscanf(idx, "%d %d %d\n", &numbers[i].idx, &numbers[i].corr, &numbers[i].guess);
	idxMax = numbers[numIdx-1].idx;
	currSearch = numbers[0].idx;
	//Seconda fase, carico le immagini.

	unsigned char** images = new unsigned char*[numIdx];
	uint32_t dump[784];
	FILE* in = fopen("train-images.idx3-ubyte", "rb");
	fread(dump, sizeof(uint32_t), 4, in);
	for (int i=0; i<=idxMax; i++)
	{
		if (i!=currSearch)
			fread(dump, sizeof(char), 784, in);
		else
		{
			printf("Leggo immagine indice %d\n", currSearch);
			images[currI] = new unsigned char[784];
			currSearch = numbers[currI+1].idx;
			for (int j=0; j<784; j++)
				images[currI][j] = fgetc(in);
			++currI;
		}
	}
	fclose(in);
	fprintf(stderr, "TUTTO OK\n");

	char fileName[255];
	
	for (int n=0; n<numIdx; n++)
	{
		printf("n: %d\n", n);
		memset(fileName, 0, 255*sizeof(char));
		sprintf(fileName, "WrongOnes/%d %d %d.xpm", numbers[n].idx, numbers[n].corr, numbers[n].guess);
		printf("Filename: %s\n", fileName);
		FILE* out = fopen(fileName, "w");
		fprintf(out, "/* XPM */\nstatic char* image[] = {\n");
		fprintf(out, "\"28 28 256 3\"\n");
		for (int i=0; i<256; i++)
			fprintf(out, "\"%s g #%s\"\n", chars[i], colors[i]);
		for (int i=0; i<28; i++)
		{
			fprintf(out, "\"");
			for (int j=0; j<28; j++)
			{
				//printf("%d ", images[n][i*28+j]);
				fprintf(out, "%s", chars[images[n][i*28+j]]);
			}
			//getchar();
			fprintf(out, "\"\n");
		}
		fprintf(out, "}");
		fflush(out);
		fclose(out);
	}
}
