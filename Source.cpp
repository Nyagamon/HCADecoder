
//--------------------------------------------------
// インクルード
//--------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "clHCA.h"
#include "HCADecodeService.h"

//--------------------------------------------------
// メイン
//--------------------------------------------------
int main(int argc, char *argv[]) {

	// コマンドライン解析
	unsigned int count = 0;
	char *filenameOut = NULL;
	//bool decodeFlg=false;
	float volume = 1;
	unsigned int ciphKey1 = 0xBC731A85;
	unsigned int ciphKey2 = 0x0002B875;
	int mode = 16;
	int loop = 0;
	bool info = false;
	bool decrypt = false;
	for (int i = 1; i<argc; i++) {
		if (argv[i][0] == '-' || argv[i][0] == '/') {
			switch (argv[i][1]) {
			case 'o':if (i + 1<argc) { filenameOut = argv[++i]; }break;
				//case 'd':decodeFlg=true;break;
			case 'v':volume = (float)atof(argv[++i]); break;
			case 'a':if (i + 1<argc) { ciphKey1 = strtoul(argv[++i], NULL, 16); }break;
			case 'b':if (i + 1<argc) { ciphKey2 = strtoul(argv[++i], NULL, 16); }break;
			case 'm':if (i + 1<argc) { mode = atoi(argv[++i]); }break;
			case 'l':if (i + 1<argc) { loop = atoi(argv[++i]); }break;
			case 'i':info = true; break;
			case 'c':decrypt = true; break;
			}
		}
		else if (*argv[i]) {
			argv[count++] = argv[i];
		}
	}

	HCADecodeService dec{}; // Start decode service

	//if(decodeFlg){

	// 入力チェック
	if (!count) {
		printf("Error: 入力ファイルを指定してください。\n");
		return -1;
	}

	// デコード
	for (unsigned int i = 0; i<count; i++) {

		// 2つ目以降のファイルは、出力ファイル名オプションが無効
		if (i)filenameOut = NULL;

		// デフォルト出力ファイル名
		char path[MAX_PATH];
		if (!(filenameOut&&filenameOut[0])) {
			strcpy_s(path, sizeof(path), argv[i]);
			char *d1 = strrchr(path, '\\');
			char *d2 = strrchr(path, '/');
			char *e = strrchr(path, '.');
			if (e&&d1<e&&d2<e)*e = '\0';
			strcat_s(path, sizeof(path), ".wav");
			filenameOut = path;
		}

		// ヘッダ情報のみ表示
		if (info) {
			printf("%s のヘッダ情報\n", argv[i]);
			clHCA hca(0, 0);
			hca.PrintInfo(argv[i]);
			printf("\n");
		}

		// 復号化
		else if (decrypt) {
			printf("%s を復号化中...\n", argv[i]);
			clHCA hca(ciphKey1, ciphKey2);
			if (!hca.Decrypt(argv[i])) {
				printf("Error: 復号化に失敗しました。\n");
			}
		}

		// デコード
		else {
			printf("%s をデコード中...\n", argv[i]);
			auto wavout = dec.decode(argv[i], 0, ciphKey1, ciphKey2, volume, mode, loop);
			if (wavout.first == nullptr)
			{
				printf("Error: デコードに失敗しました。\n");
			}
			else
			{
				FILE* outfile;
				if (fopen_s(&outfile, filenameOut, "wb"))
				{
					wavout.first; printf("Error: Failed to create WAVE file.\n");
					dec.cancel_decode(wavout.first);
					delete[] wavout.first;
				}
				else
				{
					dec.wait_on_request(wavout.first);
					fwrite(wavout.first, sizeof(unsigned char), wavout.second, outfile);
					delete[] wavout.first;
					fclose(outfile);
				}
			}
		}

	}

	//}

	return 0;
}
