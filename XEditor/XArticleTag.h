#ifndef _X_ARTICLETAG_H 
#define _X_ARTICLETAG_H

class XArticleTag : public XArticle
{
public:
	XArticleTag() :
		LineBegin(0), LineCount(0),
		LineBeginPre(0), LineCountPre(0)
	{}
	int LineBegin;
	int LineCount;
	int LineBeginPre;
	int LineCountPre;
};

#endif