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