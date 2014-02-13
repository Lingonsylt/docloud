#include "docloudfile.h"
#include "test.h"

int main(int argc, const char *argv[])
{
	doCloudFile *dcfile;
	int ret;

	dcfile = new doCloudFile;
	ret = dcfile->getFromPath("c:\\test\\docloud_TEST");
	TEST("doCloudFile->getFromPath()",(ret == 0));

	dcfile->filename = "c:\\test\\docloud_TEST";
	dcfile->blacklisted = 0;
	dcfile->updated = 12345678L;
	dcfile->uploaded = 87654321L;

	ret = dcfile->save();
	TEST("doCloudFile->save()", (ret == 0));
	TEST("doCloudFile->save() set ID", (dcfile->id != -1));

	int id = dcfile->id;
	dcfile->clear();
	TEST("doCloudFile->clear()", (dcfile->id == -1 && dcfile->filename == "" && dcfile->uploaded == 0 && dcfile->updated == 0 && dcfile->blacklisted == 0 && dcfile->matches_parent == 0));

	ret = dcfile->getFromId(id);
	TEST("doCloudFile->getFromId()", (ret == 1));
	TEST("doCloudFile->getFromId():id", (dcfile->id == id));
	TEST("doCloudFile->getFromId():filename", (dcfile->filename == "c:\\test\\docloud_TEST"));
	TEST("doCloudFile->getFromId():updated", (dcfile->updated == 12345678L));
	TEST("doCloudFile->getFromId():uploaded", (dcfile->uploaded == 87654321L));
	TEST("doCloudFile->getFromId():blacklisted", (dcfile->blacklisted == 0));
	TEST("doCloudFile->getFromId():matches_parent", (dcfile->matches_parent == 0));

	ret = dcfile->getFromPath("c:\\test\\docloud_TEST\\file_under_parent");
	TEST("doCloudFile->getFromPath():subdir",(ret == 0 && dcfile->matches_parent));


	return 0;
}

