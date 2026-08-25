/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  LogBook.h
 *  @brief: Top level class for experiment logbook.
 *  
 */
#ifndef LOGBOOK_H
#define LOGBOOK_H
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

class CSqlite;
class CSqliteException;
class LogBookPerson;
class LogBookShift;
class LogBookRun;
class LogBookNote;

/**
 * @class LogBook
 *    Top level class for log books.  This class allows clients to:
 *    -   Create a new logbook.
 *    -   Open an existing logbook
 */
class LogBook
{
public:
  /**
   * @class LogBook::Exception
   *    All errors are thrown as LogBook::Exception objects.
   *    Instances can be constructed from strings or from
   *    CSqliteException and a context string.
   */
    class Exception : public std::exception{
    private:
        std::string  m_message;
    public:
        Exception(const char* message) noexcept;
        Exception(const std::string& message) noexcept;
        Exception(const CSqliteException& reason, const char* doing) noexcept;
        Exception(const CSqliteException& reason, const std::string& doing) noexcept;
        
        Exception(const Exception& rhs) noexcept;
        Exception& operator=(const Exception& rhs) noexcept;
        
        virtual ~Exception();
        virtual const char* what() const noexcept;
        
        static void rethrowSqliteException(CSqliteException& e, const char* doing);
        static void rethrowSqliteException(CSqliteException& e, std::string doing);
    };
private:
    CSqlite* m_pConnection;
    
public:
    static std::string m_tempdir;
public:
    static void create(
       const char* pFilename, const char* pExperiment,
       const char* pSpokesperson, const char* purpose
    );
    
    
    ////
    
    LogBook(const char* pFilename);
    virtual ~LogBook();
private:
    LogBook(const LogBook& rhs);       // no copy construction.
    LogBook& operator=(const LogBook& rhs);

public:
 // Api for person
    LogBookPerson* addPerson(const char* lastName, const char* firstName, const char* salutation);
    std::vector<LogBookPerson*> findPeople(const char* where=nullptr);
    std::vector<LogBookPerson*> listPeople();
    LogBookPerson* getPerson(int id);
    
    // Api for shifts:
    
    LogBookShift* getShift(int id);
    void          addShiftMember(LogBookShift* pShift, LogBookPerson* pPerson);
    void          removeShiftMember(LogBookShift* pShift, LogBookPerson* pPerson);
    LogBookShift* createShift(const char* name);
    LogBookShift* createShift(
      const char* name, const std::vector<LogBookPerson*>& members
    );
    std::vector<LogBookShift*> listShifts();
    LogBookShift*   findShift(const char* name);
    void setCurrentShift(LogBookShift* pShift);
    LogBookShift* getCurrentShift();
    
    // API for runs:
    
    LogBookRun* getRun(int id);
    static int transitionId(CSqlite& db, const char* name);
    int         runId(int runNumber);
    LogBookRun* currentRun();
    LogBookRun* begin(int number, const char* title, const char* remark=nullptr);
    void        end(LogBookRun*& pRun, const char* remark = nullptr);
    void        pause(LogBookRun*& pRun, const char* remark = nullptr);
    void        resume(LogBookRun*& pRun, const char* remark = nullptr);
    void        emergencyStop(LogBookRun*& pRun, const char* remark = nullptr);
    std::vector<LogBookRun*> listRuns();
    LogBookRun* findRun(int number);
    
    // API for notes:
    
    LogBookNote*  createNote(
        LogBookPerson& author, const char* note, const std::vector<std::string>& imageFiles,
        const std::vector<size_t>& imageOffsets,
        LogBookRun* pRun = nullptr
    );
    LogBookNote* createNote(LogBookPerson& author, const char* note, LogBookRun* pRun = nullptr);
    LogBookNote* getNote(int id);
    std::vector<LogBookNote*> listAllNotes();
    std::vector<LogBookNote*> listNotesForRunId(int runId);
    std::vector<LogBookNote*> listNotesForRun(int runNumber);
    std::vector<LogBookNote*> listNotesForRun(const LogBookRun* pRun);
    std::vector<LogBookNote*> listNonRunNotes();
    LogBookRun* getNoteRun(LogBookNote& note);
    LogBookPerson* getNoteAuthor(LogBookNote& note);
    
    // API for key value store
    
    bool kvExists(const char* key);
    std::string kvGet(const char* key);
    void kvSet(const char* key, const char* value);
    void kvCreate(const char* key, const char* value);
    std::vector<std::pair<std::string,std::string>> kvList();
    
private:
    static std::string computeTempDir();
    static void   createSchema(CSqlite& connection);
    static void   initialize(
       CSqlite& connection,  const char* pExperiment,
       const char* pSpokesperson, const char* purpose
    );
    static void stockValidTransitions(CSqlite& db);
    static void stockStateMachine(CSqlite& db);
};


#endif