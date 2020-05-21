#ifndef LOGFILE_H
#define LOGFILE_H

#ifdef __cplusplus
extern "C" {
#endif
	void LogFileOpen(const char *filename);
	void LogFileWrite(const char *text, ...);
	void LogFileClose();
#ifdef __cplusplus
}
#endif

#endif //LOGFILE_H