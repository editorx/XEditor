#ifndef _ARTICLE_H_
#define _ARTICLE_H_

class XArticle
{
public:
	struct SENTENCE
	{
		SENTENCE *Prev;
		SENTENCE *Next;
		int count;
		TCHAR *content;
		BOOL searchWord(TCHAR *word, int startPos, int *index, BOOL CaseSensitive);
		BOOL searchWordRev(TCHAR *word, int startPos, int *index, BOOL CaseSensitive);
		BOOL searchWholeWord(TCHAR *word, int startPos, int *index, BOOL CaseSensitive);
		BOOL searchWholeWordRev(TCHAR *word, int startPos, int *index, BOOL CaseSensitive);
	};

	TCHAR *pBuf;
	TCHAR *pBufSave;
	
private:
	SENTENCE *pHead;
	SENTENCE *pTail;
	int rows;
	int size;
public:
	XArticle();
	XArticle(TCHAR *content);
	~XArticle();

	void InsertRow(int line, TCHAR *content);
	void InsertRowBegin(TCHAR *content);
	void InsertRowEnd(TCHAR *content);
	void DeleteRow(int line);
	void DeleteRowBegin();
	void DeleteRowEnd();
	void InsertChar(int line, int col, TCHAR ch);
	void InsertCharBegin(int line, TCHAR ch);
	void InsertCharEnd(int line, TCHAR ch);
	void DeleteChar(int line, int col);
	void DeleteCharBegin(int line);
	void DeleteCharEnd(int line);
	void InsertString(int line, int col, TCHAR *str, int len);
	void InsertStringBegin(int line, TCHAR *str, int len);
	void InsertStringEnd(int line, TCHAR *str, int len);
	void DeleteString(int line, int col, int len);
	void DeleteStringBegin(int line, int len);
	void DeleteStringEnd(int line, int len);

	void InsertMultiString(int line, int col, TCHAR *str, int len);
	void DeleteMultiString(int y0, int x0, int y1, int x1);
	void CopyMultiString(TCHAR *dest, int x0, int y0, int x1, int y1, int &num);

	int GetSize();
	int GetRows();
	int GetCount(int line);
	TCHAR *GetContent(int line);
	int GetRealLength(int x0, int y0, int x1, int y1);
	void CopyString(TCHAR *dest, int line, int x0, int x1);

	void CombineRow(int line);
	void SeparateRow(int line, int col);

	int GetColMax();
	int GetCol(int line);
	int GetCol(int line, int index);
	int GetDataIndex(int line, int col);
	TCHAR GetData(int line, int col);
	TCHAR GetDataByIndex(int line, int index);
	BOOL GetDataType(TCHAR ch);

	BOOL SearchText(int line, int startPos, TCHAR *text, int *index,
		BOOL CaseSensitive, BOOL WholeWord);
	BOOL SearchTextRev(int line, int startPos, TCHAR *text, int *index,
		BOOL CaseSensitive, BOOL WholeWord);

	TCHAR *Format();

	void Initialize(TCHAR *Src);
	void Destory();
	void Print();
};

#endif
