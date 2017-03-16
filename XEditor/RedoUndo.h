#ifndef _REDO_UNDO_H
#define _REDO_UNDO_H

struct LogElement
{
	enum CMD
	{
		DEFAULT = 0,
		ADD_CHAR = 1,
		DEL_CHAR = 2,
		BACKSPACE = 3,
		ENTER = 4,
		COPY = 5,
		PASTE = 6,
		CUT = 7,

		OFFSET = 100,   //分隔符

		UN_ADD_CHAR = 101,
		UN_DEL_CHAR = 102,
		UN_BACKSPACE = 103,
		UN_ENTER = 104,
		UN_COPY = 105,
		UN_PASTE = 106,
		UN_CUT = 107
	}cmd;
	int line;			 //保存行信息
	int index;           //保存列信息
	int lineCount;       //行数
};

enum RedoOrUndo
{
	REDO,
	UNDO
};

#endif