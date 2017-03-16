#include <stdio.h>
#include "common.h"
#include "XEditor.h"

XEditor::XEditor(HINSTANCE hInstance)
{
	m_hInst = hInstance;
	InitFilePath[0] = TEXT('\0');
}

XEditor::~XEditor() {}

