#include <windows.h>
#include <stdio.h>
#include "XArticle.h"

BOOL XArticle::SearchText(int line, int startPos, TCHAR *text, int *index, BOOL CaseSensitive, BOOL WholeWord)
{
	SENTENCE *pNode = pHead;
	for (int i = 0; i < line; i++)
		pNode = pNode->Next;

	if (CaseSensitive)
	{
		if (WholeWord)
		{
			return pNode->searchWholeWord(text, startPos, index, TRUE);
		}
		else
		{
			return pNode->searchWord(text, startPos, index, TRUE);
		}
	}
	else
	{
		if (WholeWord)
		{
			return pNode->searchWholeWord(text, startPos, index, FALSE);
		}
		else
		{
			return pNode->searchWord(text, startPos, index, FALSE);
		}
	}
}

BOOL XArticle::SearchTextRev(int line, int startPos, TCHAR *text, int *index, BOOL CaseSensitive, BOOL WholeWord)
{
	SENTENCE *pNode = pHead;
	for (int i = 0; i < line; i++)
		pNode = pNode->Next;

	if (CaseSensitive)
	{
		if (WholeWord)
		{
			return pNode->searchWholeWordRev(text, startPos, index, TRUE);
		}
		else
		{
			return pNode->searchWordRev(text, startPos, index, TRUE);
		}
	}
	else
	{
		if (WholeWord)
		{
			return pNode->searchWholeWordRev(text, startPos, index, FALSE);
		}
		else
		{
			return pNode->searchWordRev(text, startPos, index, FALSE);
		}
	}
}

BOOL XArticle::SENTENCE::searchWord(TCHAR *word, int startPos, int *index, BOOL CaseSensitive)
{
	TCHAR *srcData = word;
	int srcLen = lstrlen(srcData);
	TCHAR *content = this->content + startPos;      //偏移量计算在内
	int count = this->count - startPos;

	if (srcLen > count)
		return FALSE;

	BOOL isFind = FALSE;

	for (int i = 0; i < (count - srcLen + 1); i++)
	{
		for (int j = 0; j < srcLen; j++)
		{
			if (CaseSensitive)
			{
				if(srcData[j] != content[i+j])
					break;
			}
			else
			{
				if(toupper(srcData[j] != toupper(content[i+j])))
					break;
			}
			if (j == srcLen - 1)
			{
				*index = i + startPos;
				isFind = TRUE;
			}
		}
		if (isFind == TRUE)
		{
			break;
		}
	}

	return isFind;
}

BOOL XArticle::SENTENCE::searchWordRev(TCHAR *word, int startPos, int *index, BOOL CaseSensitive)
{
	TCHAR *srcData = word;
	int srcLen = lstrlen(srcData);
	TCHAR *content = this->content;      //偏移量计算在内
	int count = startPos;

	if (srcLen > count)
		return FALSE;

	BOOL isFind = FALSE;

	for (int i = (count - srcLen); i >= 0; i--)
	{
		for (int j = 0; j < srcLen; j++)
		{
			if (CaseSensitive)
			{
				if (srcData[srcLen - j + 1] != content[i + (srcLen - j - 1)])
					break;
			}
			else
			{
				if (toupper(srcData[srcLen - j + 1] != toupper(content[i + (srcLen - j - 1)])))
					break;
			}
			if (j == srcLen - 1)
			{
				*index = i;
				isFind = TRUE;
			}
		}
		if (isFind == TRUE)
		{
			break;
		}
	}

	return isFind;
}

BOOL XArticle::SENTENCE::searchWholeWord(TCHAR *word, int startPos, int *index, BOOL CaseSensitive)
{
	TCHAR *srcData = word;
	int srcLen = lstrlen(srcData);
	TCHAR *content = this->content + startPos;      //偏移量计算在内
	int count = this->count - startPos;

	if (srcLen > count)
		return FALSE;

	BOOL isFind = FALSE;

	for (int i = 0; i < (count - srcLen + 1); i++)
	{

		if (i == 0 || (i > 0 && content[i - 1] == TEXT(' ')))
		{
			for (int j = 0; j < srcLen; j++)
			{
				if (CaseSensitive)
				{
					if (srcData[j] != content[i + j])
						break;
				}
				else
				{
					if (toupper(srcData[j] != toupper(content[i + j])))
						break;
				}
				if (j == srcLen - 1)
				{
					if(content[i + j + 1] == TEXT(' ')
						|| content[i + j + 1] == TEXT('\0'))
					{
						*index = i + startPos;
						isFind = TRUE;
					}
				}
			}
		}
		if (isFind == TRUE)
		{
			break;
		}
	}

	return isFind;
}

BOOL XArticle::SENTENCE::searchWholeWordRev(TCHAR *word, int startPos, int *index, BOOL CaseSensitive)
{
	TCHAR *srcData = word;
	int srcLen = lstrlen(srcData);
	TCHAR *content = this->content;      //偏移量计算在内
	int count = startPos;

	if (srcLen > count)
		return FALSE;

	BOOL isFind = FALSE;

	for (int i = (count - srcLen); i >= 0; i--)
	{
		if (i == (count - srcLen) || (i < (count - srcLen) && content[i + srcLen] == TEXT(' ')))
		{
			for (int j = 0; j < srcLen; j++)
			{
				if (CaseSensitive)
				{
					if (srcData[srcLen - j + 1] != content[i + (srcLen - j - 1)])
						break;
				}
				else
				{
					if (toupper(srcData[srcLen - j + 1] != toupper(content[i + (srcLen - j - 1)])))
						break;
				}
				if (j == srcLen - 1)
				{
					if (i == 0
						|| content[i - 1] == TEXT(' '))
					{
						*index = i;
						isFind = TRUE;
					}
				}
			}
		}
		if (isFind == TRUE)
		{
			break;
		}
	}

	return isFind;
}

void XArticle::CopyMultiString(TCHAR *dest, int x0, int y0, int x1, int y1, int &num)
{
	int temp = 0;
	if (y0 > y1)
	{
		temp = y0; y0 = y1; y1 = temp;
		temp = x0; x0 = x1; x1 = temp;
	}
	else if(y0 == y1 && x0 > x1)
	{ 
		temp = x0; x0 = x1; x1 = temp;
	}
	if (y0 == y1)
	{
		CopyString(dest, y0, x0, x1);
		dest += x1 - x0;
		*dest++ = TEXT('\0');
	}
	else
	{
		for (int i = 0; i < y1 - y0 + 1; i++)
		{
			if (i == 0)
			{
				CopyString(dest, y0, x0, GetCount(y0));
				dest += GetCount(y0) - x0;
				*dest++ = TEXT('\r');
				*dest++ = TEXT('\n');
			}
		}
	}
	num = 0;
	if (y0 == y1)
	{
		num = x1 - x0;
	}
	else
	{
		for (int i = 0; i < y1 - y0 + 1; i++)
		{
			if (i == 0)
			{
				num += GetCount(y0)-x0;
				num += 2;
			}
			else if(i == y1 - y0)
			{
				num += x1;
			}
			else
			{
				num += GetCount(i + y0);
				num += 2;
			}
		}
	}
}

int XArticle::GetRealLength(int x0, int y0, int x1, int y1)
{
	int num = 0;
	if (y0 == y1)
	{
		num = x1 - x0;
	}
	else
	{
		for (int i = 0; i < y1 - y0 + 1; i++)
		{
			if (i == 0)
			{
				num += GetCount(y0) - x0;
				num += 2;
			}
			else if (i == y1 - y0)
			{
				num += x1;
			}
			else
			{
				num += GetCount(i + y0);
				num += 2;
			}
		}
	}
	return num;
}

void XArticle::CopyString(TCHAR *dest, int line, int x0, int x1)
{
	TCHAR *content = GetContent(line);
	int length = x1 - x0;
	if (length <= 0)
		return;
	for (int i = 0; i < length; i++)
	{
		dest[i] = content[x0 + i];
	}
}

void XArticle::InsertMultiString(int line, int col, TCHAR *str, int len)
{
	TCHAR *pBegin = str;
	TCHAR *pEnd = str;
	bool MultiFlag = false;
	int lastLinePos = 0;
	for (int i = 0; i < len; i++)
	{
		if (*pEnd == TEXT('\r'))
		{
			if (MultiFlag == false)
			{
				MultiFlag = true;
				int lenTemp = pEnd - pBegin;
				SeparateRow(line, col);
				InsertString(line, col, pBegin, lenTemp);
				line++;
			}
			else
			{
				int lenTemp = pEnd - pBegin;
				InsertRow(line, TEXT('\0'));
				InsertString(line, 0, pBegin, lenTemp);
				line++;
			}
			pEnd++;
			continue;
		}
		if (*pEnd == TEXT('\n'))
		{
			pEnd++;
			pBegin = pEnd;
			lastLinePos = pEnd - str;
			continue;
		}
		pEnd++;
	}
	if (MultiFlag == true && lastLinePos < len)
	{
		InsertString(line, 0, str + lastLinePos, len - lastLinePos);
	}
	if (MultiFlag == false)
	{
		InsertString(line, col, str, len);
	}
}

void XArticle::DeleteMultiString(int y0, int x0, int y1, int x1)
{
	int temp;
	if (y0 > y1 || (y0 == y1 && x0 > x1))            // 如果顺序反了则交换
	{
		temp = y0; y0 = y1; y1 = temp;
		temp = x0; x0 = x1; x1 = temp;
	}
	if (y1 - y0 > 1)
	{
		for (int i = 0; i < (y1 - y0 - 1); i++)
		{
			DeleteRow(y0 + 1);
		}
	}
	else if (y1 == y0)
	{
		DeleteString(y0, x0, x1 - x0);
		return;
	}
	int length = GetCount(y0) - x0;
	for (int i = 0; i < length; i++)
	{
		DeleteCharEnd(y0 + 1);
	}
	for (int i = 0; i < x1; i++)
	{
		DeleteCharBegin(y0 + 1);
	}
	CombineRow(y0);
}

void XArticle::CombineRow(int line)
{
	InsertStringEnd(line, GetContent(line + 1), GetCount(line + 1));
	DeleteRow(line + 1);
}

void XArticle::SeparateRow(int line, int col)
{
	int ColEnd = GetCount(line);
	int lengthCut = ColEnd - col;
	TCHAR *str = GetContent(line);
	InsertRow(line + 1, TEXT('\0'));
	InsertString(line + 1, 0, str + col, lengthCut);
	DeleteString(line, col, lengthCut);
}

TCHAR *XArticle::GetContent(int line)
{
	SENTENCE *pNode = pHead;
	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	return pNode->content;
}

int XArticle::GetSize()
{
	int realSize;
	if (rows > 0)
		realSize = size + (rows - 1) * 2;
	else
		realSize = size;
	return realSize;
}

int XArticle::GetRows()
{
	return rows;
}

int XArticle::GetCount(int line)
{
	SENTENCE *pNode = pHead;
	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	return pNode->count;
}

void XArticle::DeleteString(int line, int col, int len)
{
	for (int i = 0; i < len; i++)
	{
		DeleteChar(line, col);
	}
}

void XArticle::DeleteStringBegin(int line, int len)
{
	for (int i = 0; i < len; i++)
	{
		DeleteCharBegin(line);
	}
}

void XArticle::DeleteStringEnd(int line, int len)
{
	for (int i = 0; i < len; i++)
	{
		DeleteCharEnd(line);
	}
}

void XArticle::InsertString(int line, int col, TCHAR *str, int len)
{
	for (int i = 0; i < len; i++)
	{
		InsertChar(line, col, str[len - i - 1]);
	}
}

void XArticle::InsertStringBegin(int line, TCHAR *str, int len)
{
	for (int i = 0; i < len; i++)
	{
		InsertCharBegin(line, str[len - i - 1]);
	}
}

void XArticle::InsertStringEnd(int line, TCHAR *str, int len)
{
	for (int i = 0; i < len; i++)
	{
		InsertCharEnd(line, str[len - i - 1]);
	}
}

int XArticle::GetColMax()
{
	SENTENCE *pNode = pHead;
	int length = 0;
	int temp = 0;
	for (int i = 0; i < rows; i++)
	{
		temp = 0;
		for (int j = 0; j < pNode->count; j++)
		{
			if (*(pNode->content + j) > 0x00FF)         //全角字符多加一次
			{
				temp++;
			}
			temp++;
		}
		
		if (length < temp)
		{
			length = temp;
		}
		pNode = pNode->Next;
	}
	return length;
}

int XArticle::GetCol(int line)
{
	SENTENCE *pNode = pHead;
	int length = 0;

	if (line >= rows)
	{
		line = rows - 1;
	}

	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	for (int j = 0; j < pNode->count; j++)
	{
		if (*(pNode->content + j) > 0x00FF)   // 全角字符多加一次
		{
			length++;
		}
		length++;
	}
	return length;
}

//col == index + num(pNode->content > 0x00FF)

int XArticle::GetCol(int line, int index)
{
	SENTENCE *pNode = pHead;
	int length = 0;

	if (line >= rows)
	{
		line = rows - 1;
	}

	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}

	if (index > pNode->count)
	{
		pNode = pNode->Next;
	}
	for (int j = 0; j < index; j++)
	{
		if (*(pNode->content + j) > 0x00FF)   // 全角字符多加一次
		{
			length++;
		}
		length++;
	}
	return length;
}

int XArticle::GetDataIndex(int line, int col)
{
	SENTENCE *pNode = pHead;
	int length = 0;
	int index = 0;

	if (line >= rows)
	{
		line = rows - 1;
	}

	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	for (int j = 0; j < pNode->count; j++)
	{
		if (col <= length)            
		{
			index = j;
			break;
		}
		if (col != 0 && index == 0 && j == pNode->count - 1)
		{
			index = j + 1;
			break;
		}
		if (*(pNode->content + j) > 0x00FF)
		{
			length++;
		}
		length++;
	}
	return index;
}

TCHAR XArticle::GetData(int line, int col)
{
	SENTENCE *pNode = pHead;
	int length = 0;
	TCHAR dat = TEXT('\0');
	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	for (int j = 0; j < pNode->count; j++)
	{
		if (col <= length)
		{
			dat = *(pNode->content + j);
			break;
		}
		if (*(pNode->content + j) > 0x00FF)
		{
			length++;
		}
		length++;
	}
	return dat;
}

TCHAR XArticle::GetDataByIndex(int line, int index)
{
	SENTENCE *pNode = pHead;
	int length = 0;
	TCHAR dat = TEXT('\0');
	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	if (index < 0)
	{
		index = 0;
	}
	if (index >= pNode->count)
	{
		index = pNode->count;
	}

	dat = *(pNode->content + index);
	return dat;
}

BOOL XArticle::GetDataType(TCHAR ch)
{
	if (ch > 0x00FF)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

TCHAR *XArticle::Format()
{
	int wholeSize = 0;
	wholeSize = this->size + 2 * rows + 1;
	if (pBufSave != NULL)
		free(pBufSave);
	pBufSave = (TCHAR *)malloc(wholeSize * sizeof(wholeSize));
	TCHAR *pBufTemp = pBufSave;
	SENTENCE *pNode = pHead;
	for (int i = 0; i < rows - 1; i++)
	{
		lstrcpy(pBufSave, pNode->content);
		pBufSave += pNode->count;
		lstrcpy(pBufSave, TEXT("\r\n"));
		pBufSave += 2;
		pNode = pNode->Next;
	}
	lstrcpy(pBufSave, pNode->content);
	pBufSave = pBufTemp;
	pBufSave[wholeSize - 1] = TEXT('\0');
	return pBufSave;
}

void XArticle::InsertChar(int line, int col, TCHAR ch)
{
	if (line < 0 || line > rows)
		return;

	SENTENCE *pNode = pHead;
	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	TCHAR *pText = pNode->content;
	int sLen = (pNode->count + 1) + 1;
	if (col > sLen - 2)
		return;
	TCHAR *pTemp = (TCHAR *)malloc(sLen * sizeof(TCHAR));
	int index = 1;
	for (index = 0; index < col; index++)
	{
		pTemp[index] = pText[index];
	}
	pTemp[index] = ch;
	index++;
	for (; index < sLen; index++)
	{
		pTemp[index] = pText[index - 1];
	}
	pNode->content = pTemp;
	pNode->count++;
	size++;
	free(pText);
}

void XArticle::InsertCharBegin(int line, TCHAR ch)
{
	InsertChar(line, 0, ch);
}

void XArticle::InsertCharEnd(int line, TCHAR ch)
{
	SENTENCE *pNode = pHead;
	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	TCHAR *pText = pNode->content;
	int sLen = (pNode->count + 1) + 1;
	InsertChar(line, sLen - 2, ch);
}

void XArticle::DeleteChar(int line, int col)
{
	if (line < 0 || line >= rows)
		return;

	SENTENCE *pNode = pHead;
	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	TCHAR *pText = pNode->content;
	int sLen = (pNode->count + 1) - 1;
	if (col > sLen - 1)
		return;
	TCHAR *pTemp = (TCHAR *)malloc(sLen * sizeof(TCHAR));
	int index = 0;
	for (index = 0; index < col; index++)
	{
		pTemp[index] = pText[index];
	}
	for ( ; index < sLen; index++)
	{
		pTemp[index] = pTemp[index + 1];
	}
	pNode->content = pTemp;
	pNode->count--;
	size--;
	free(pText);
}

void XArticle::DeleteCharBegin(int line)
{
	DeleteChar(line, 0);
}

void XArticle::DeleteCharEnd(int line)
{
	SENTENCE *pNode = pHead;
	for (int i = 0; i < line; i++)
	{
		pNode = pNode->Next;
	}
	TCHAR *pText = pNode->content;
	int sLen = (pNode->count + 1) - 1;
	DeleteChar(line, sLen - 1);
}

void XArticle::Print()
{
	SENTENCE *pNode = pHead;
	while (pNode->Next != NULL)
	{
		wprintf(pNode->content);
		wprintf(TEXT("\t\tLength = %d"), pNode->count);
		wprintf(TEXT("\n"));
		pNode = pNode->Next;
	}
}

void XArticle::InsertRow(int line, TCHAR *content)
{
	if (line < 0 || line > rows)
		return;
	SENTENCE *pNode = (SENTENCE *)malloc(sizeof(SENTENCE));
	int length = lstrlen(content);
	size += length;
	pNode->content = (TCHAR *)malloc((length + 1) * sizeof(TCHAR));
	pNode->count = length;
	if (length != 0)
	{
		lstrcpy(pNode->content, content);
	}
	pNode->content[length] = TEXT('\0');
	if (line == 0)
	{
		pNode->Prev = NULL;
		pNode->Next = pHead;
		pHead->Prev = pNode;
		pHead = pNode;
	}
	else if (line == rows)
	{
		pNode->Prev = pTail->Prev;
		pTail->Prev->Next = pNode;
		pNode->Next = pTail;
		pTail->Prev = pNode;
	}
	else
	{
		SENTENCE *pTime = pHead;
		for (int i = 0; i < line; i++)
		{
			pTime = pTime->Next;
		}
		pNode->Next = pTime;
		pNode->Prev = pTime->Prev;
		pTime->Prev->Next = pNode;
		pTime->Prev = pNode;
	}
	rows++;
}

void XArticle::InsertRowBegin(TCHAR *content)
{
	InsertRow(0, content);
}

void XArticle::InsertRowEnd(TCHAR *content)
{
	InsertRow(rows, content);
}

void XArticle::DeleteRow(int line)
{
	if (line < 0 || line > rows - 1)
		return;
	SENTENCE *pNode;
	int length;
	if (line == 0)
	{
		pNode = pHead;
		pHead = pHead->Next;
		pHead->Prev = NULL;
		length = pNode->count;
		free(pNode->content);
		free(pNode);
	}
	else if (line == rows - 1)
	{
		pNode = pTail->Prev;
		length = pNode->count;
		pNode->Prev->Next = pTail;
		pTail->Prev = pNode->Prev;
		free(pNode->content);
		free(pNode);
	}
	else
	{
		pNode = pHead;
		for (int i = 0; i < line; i++)
		{
			pNode = pNode->Next;
		}
		pNode->Prev->Next = pNode->Next;
		pNode->Next->Prev = pNode->Prev;
		length = pNode->count;
		free(pNode->content);
		free(pNode);
	}
	rows--;
	size -= length;
}

void XArticle::DeleteRowBegin()
{
	DeleteRow(0);
}

void XArticle::DeleteRowEnd()
{
	DeleteRow(rows - 1);
}

XArticle::XArticle() : rows(0), size(0)
{
	pBuf = NULL;
	pBufSave = NULL;
	pHead = NULL;
	pTail = NULL;
	rows = 0;
}

XArticle::XArticle(TCHAR *SrcData) : rows(0), size(0)
{
	pBuf = NULL;
	pBufSave = NULL;
	pHead = NULL;
	pTail = NULL;
	rows = 0;
	Initialize(SrcData);
}

XArticle::~XArticle()
{
	Destory();
}

void XArticle::Initialize(TCHAR *Src)
{
	int length = lstrlen(Src) + 1;
	pBuf = (TCHAR *)malloc(length * sizeof(TCHAR));
	lstrcpy(pBuf, Src);

	pTail = pHead = NULL;
	int sLen = lstrlen(pBuf);
	TCHAR *pBegin = pBuf;
	TCHAR *pEnd = pBuf;
	size = 0;
	rows = 0;
	for (int i = 0; i < sLen + 1; i++)\
	{
		if (*pEnd == TEXT('\r') || *pEnd == TEXT('\0'))
		{
			int lenTemp = pEnd - pBegin;
			size += lenTemp;
			TCHAR *strTemp = (TCHAR *)malloc((lenTemp + 1) * sizeof(TCHAR));
			for (int j = 0; j < lenTemp; j++)
			{
				strTemp[j] = *pBegin++;
			}
			strTemp[lenTemp] = TEXT('\0');
			if (pHead == NULL)
			{
				pHead = (SENTENCE *)malloc(sizeof(SENTENCE));
				pTail = (SENTENCE *)malloc(sizeof(SENTENCE));
				pHead->Prev = NULL;
				pHead->Next = pTail;
				pHead->content = strTemp;
				pHead->count = lenTemp;
				pTail->Prev = pHead;
				pTail->Next = NULL;
				pTail->content = (TCHAR *)malloc(sizeof(TCHAR));
				pTail->content[0] = TEXT('\0');
			}
			else
			{
				SENTENCE *pNode = (SENTENCE *)malloc(sizeof(TCHAR));
				pNode->Prev = pTail->Prev;
				pTail->Prev->Next = pNode;
				pNode->Next = pTail;
				pTail->Prev = pNode;
				pNode->content = strTemp;
				pNode->count = lenTemp;
			}
			pEnd++;
			rows++;
			continue;
		}
		if (*pEnd == TEXT('\n'))
		{
			pEnd++;
			pBegin = pEnd;
			continue;
		}
		pEnd++;
	}
}

void XArticle::Destory()
{
	if (pHead != NULL)
	{
		while (pHead->Next != NULL)
		{
			SENTENCE *pNode = pHead;
			pHead = pHead->Next;
			free(pNode->content);
			free(pNode);
		}
		free(pHead->content);
		free(pHead);
	}
	if (pBuf != NULL)
	{
		free(pBuf);
		pBuf = NULL;
	}
	if (pBufSave != NULL)
	{
		free(pBufSave);
		pBufSave = NULL;
	}
	pHead = NULL;
	pTail = NULL;
	rows = 0;
	size = 0;
}