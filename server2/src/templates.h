#ifndef _SFMTEMPLATES_H                       // duplication check
#define _SFMTEMPLATES

#include <ctemplate/template.h>

using namespace ctemplate;

namespace superfastmatch{
	// Register standard includes
	RegisterTemplateFilename(HEADER, "header.tpl");
	RegisterTemplateFilename(FOOTER, "footer.tpl");

	// Register pages
	RegisterTemplateFilename(STATUS_PAGE, "status_page.tpl");
	RegisterTemplateFilename(INDEX_PAGE, "index_page.tpl");

	// Register parts
	RegisterTemplateFilename(POSTING_STATS, "posting_stats.tpl");
}

#endif

