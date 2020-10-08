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

/** @file:  LogBookNote.cpp
 *  @brief: Impelement the log book note class.
 */
#include "LogBookNote.h"
#include "LogBookRun.h"
#include "LogBook.h"

#include <CSqlite.h>
#include <CSqliteException.h>
#include <CSqliteStatement.h>
#include <CSqliteTransaction.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#include <sstream>
#include <map>
#include <fstream>

////////////////////////////////////////////////////////////////
// NoteImageCanonicals.

/**
 * DefaultConstructor
 *    Just initialize the image information:
 */
LogBookNote::_NoteImage::_NoteImage() :
    s_imageLength(0),
    s_pImageData(nullptr)
{}
/**
 * copy constructor:
 *
 * @param rhs - the object we're copying.
 */
LogBookNote::_NoteImage::_NoteImage(const _NoteImage& rhs) :
    s_pImageData(0), s_imageLength(0)
{
    CopyIn(rhs);
}
/**
 * destructor:
 */
LogBookNote::_NoteImage::~_NoteImage()
{
    free(s_pImageData);            // No-op if nullptr.
}
/**
 * assignment
 *    @param rhs - right hand side of the assignment.
 *    @return *this
 */
LogBookNote::_NoteImage&
LogBookNote::_NoteImage::operator=(const _NoteImage& rhs)
{
    if (this != &rhs) {
        CopyIn(rhs);
    }
    return *this;
}
/**
 * CopyIn
 *   Common code to copy from a rhs object to this.
 * @param rhs - What we copy from
 */
void
LogBookNote::_NoteImage::CopyIn(const _NoteImage& rhs)
{
    s_id               = rhs.s_id;
    s_noteId           = rhs.s_noteId;
    s_noteOffset       = rhs.s_noteOffset;
    s_originalFilename = rhs.s_originalFilename;
    s_imageLength      = rhs.s_imageLength;
    free(s_pImageData);
    s_pImageData = malloc(s_imageLength);
    if (!s_pImageData) {
        throw LogBook::Exception(
            "Copying a NoteImage - unable to malloc image data"
        );
    }
    memcpy(s_pImageData, rhs.s_pImageData, s_imageLength);
}
////////////////////////////////////////////////////////////////
// LogBookNote implementation

/**
 * constructor
 *     This constructor constructs the note from the information
 *     about it in the database.
 *  @param db  - references the database connection object.
 *  @param id  - The primary key value of the note.
 */
LogBookNote::LogBookNote(CSqlite& db, int noteId)
{
    bool found(false);
    try {
        CSqliteStatement find(
            db,
            "SELECT run_id, note_time, note,                           \
                    note_image.id AS image_id, note_offset,            \
                    original_filename, image                           \
            FROM note                                                  \
            LEFT JOIN note_image ON note.id = note_id                  \
            WHERE note.id = ?"
        );
        find.bind(1, noteId);
        
        // Notes may not have images  in which case the fields
        // from note_image will be nulls; and we'll only get one
        // returned record.  Otherwise we'll get one returned record
        // for each image.
       
        while (!(++find).atEnd()) {
            found = true;
            m_textInfo.s_id    = noteId;
            m_textInfo.s_runId = find.getInt(0);
            m_textInfo.s_noteTime = find.getInt(1);
            m_textInfo.s_contents = find.getString(2);
            
            // We need to be sure we have image data
            // If so we emplace an image into the s_imageInfo
            // field of m_Note and fill it in from the data
            // in the record.  Note that the blob contents are
            // gotten as a pointer that's valid only until the next
            // ++find so we need to malloc/copy the data:
            
            if (find.columnType(3) != CSqliteStatement::null) {
                m_imageInfo.emplace_back();
                NoteImage& image(m_imageInfo.back());
                image.s_id = find.getInt(3);
                image.s_noteId = noteId;
                image.s_noteOffset = find.getInt(4);
                image.s_originalFilename = find.getString(5);
                image.s_imageLength      = find.bytes(6);
                image.s_pImageData       = malloc(image.s_imageLength);
                if (!image.s_pImageData) {
                    throw LogBook::Exception(
                        "Failed allocating storage for an image in LogbookNote constructor"
                    );
                } else {
                    memcpy(
                        image.s_pImageData, find.getBlob(6),
                        image.s_imageLength
                    );
                }
            }
            
        }
    }
    catch (CSqliteException& e) {
        LogBook::Exception::rethrowSqliteException(
            e, "Failed to lookupt note"
        );
    }
    
    // If found is still false, this is an error:
    
    if (!found) {
        std::stringstream msg;
        msg << "There is no note with the primary key value " << noteId;
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
}
/**
 * copy construction
 *
 * @param rhs - the object we're copy constructing.
 */
LogBookNote::LogBookNote(const LogBookNote& rhs)
{
    CopyIn(rhs);
}

/** Assignment
 *    @param rhs - object we're assigned to.
 *    @return LogBookNote& (*this)
 */
LogBookNote&
LogBookNote::operator=(const LogBookNote& rhs)
{
    if (this != &rhs) {
        m_imageInfo.clear();
        CopyIn(rhs);
    }
    return *this;
}
/**
 * destructor
 */
LogBookNote::~LogBookNote()
{
    // Destructors of the images take care of the malloc'd data.
}
/**
 * getAssociatedRun
 *   Return an object encapsulating the run that this note is on.
 *   If the run number is null (0) valued. Then  a nullptr is returned.
 * @param db - database.
 * @return LogBookRun* - the run -dynamically allocated.
 * @retval nullptr     - the note is not associated with any run.
 * @throw LogBook::Exception on other errors.
 */
LogBookRun*
LogBookNote::getAssociatedRun(CSqlite& db) const
{
    if (m_textInfo.s_runId == 0) {
        return nullptr;
    }
    return new LogBookRun(db, m_textInfo.s_runId);
}
/**
 * getNoteText
 *   Returns information about the raw note text.
 * @return const LogBookNote::NoteText& - const reference
 *        to information about the note text.
 */
const LogBookNote::NoteText&
LogBookNote::getNoteText() const
{
    return m_textInfo;
}
/**
 * imageCount
 *    Returns the number of images associated with the note.
 *    Information on the images themselves can be retrieved
 *    using  the indexing operation (operator[]).
 * @return size_t
 */
size_t
LogBookNote::imageCount() const
{
    return m_imageInfo.size();
}
/**
 * operator[]
 *    Returns a const reference to the image information about
 *    the indexed image.
 *
 * @param int - index.
 * @return const LogBookNote::NoteIMage&
 * @note at is used so std::out_of_range will be thrown if
 *       the index is invalid.
 */
const LogBookNote::NoteImage&
LogBookNote::operator[](int n) const
{
    return m_imageInfo.at(n);
}
/**
 * substituteImages
 *    This is by far the most complex member method in the class;
 *    - If there are no images, we can just return the note text.
 *    - If there are images we need to get the images sorted by
 *      increasing character index in the input file.
 *    For each image, we copy the text between the previous image's
 *    link end to the next image.  We then need to parse the link to
 *    ensure it's correct. We need to extract the image into our temp
 *    directory with a nice filename and generate, in the output
 *    new link information that points to that nice filename.
 *    next skip the link and continue copying.
 * @return std::string - the markdown of the note text with
 *                       image links appropriately edited.
 * @note image files will be written into the temp directory with
 *       a predicatable name based on the tail of their filename,
 *       the note id, and the image id.  This is unlikely to collide
 *       with other images in e.g. other notebooks in case
 *       we later decide to conditionally copy only if the image
 *       isn't already cached.
 */
std::string
LogBookNote::substituteImages()
{
    // special case where no substitutions are needed:
    
    if (imageCount() == 0) {
        return m_textInfo.s_contents;   // Done.
    }
    
    std::string result;
    
    // Cheap sort by creating a map that is indexed by offset
    // and contains pointers to the NoteImage data for that offset:
    
    std::map<int, pNoteImage> images;
    for (int i =0; i < imageCount(); i++) {
        images[m_imageInfo[i].s_noteOffset] = &(m_imageInfo[i]);
    }
    size_t cursor(0);       // Where we are in the original text.
    for (auto p =  images.begin(); p != images.end(); p++) {
        int linkIndex = p->first;
        NoteImage& image(*(p->second));
        
        // Copy from cursor to the link into the output:
        
        result += m_textInfo.s_contents.substr(cursor, (linkIndex-cursor));
        LinkInfo link = parseLink(image);
        std::string filename = exportImage(image);
        result += editLink(link, filename);
        
        cursor = linkIndex + link.s_length;
        
    }
    result += m_textInfo.s_contents.substr(cursor);  // Any remaining text.
    
    return result;
}
///////////////////////////////////////////////////////////////
// Static members:

/**
 * create(static)
 *    Create a new logbook note:
 *
 *  @param db    - references the database connection object
 *  @param run   - Pointer to run information.  This can be null
 *                 if the note is not associated with any run.
 *  @param string - The markdown infested note string.
 *  @param images - Descriptions of images to associate with the
 *                  the note.
 *  @return LogBookNote*  - Pointer to the created note. Note that all
 *                  insertions are done  in a transaction to make the
 *                  insertion atomic with respect to database clients.
 */
LogBookNote*
LogBookNote::create(
    CSqlite& db, LogBookRun* run, const char* string,
    const std::vector<ImageInfo>& images
)
{
    LogBookNote* pResult(nullptr);
    CSqliteTransaction t(db);
    try {
        // Root record.
        
        CSqliteStatement insroot(
            db,
            "INSERT INTO note (run_id, note_time, note)      \
                VALUES(?,?,?)"
        );
        // Only bind the run id if there's an associated run:
        
        if (run) {
            insroot.bind(1, run->getRunInfo().s_id);
        }
        
        insroot.bind(2, time_t(nullptr));
        insroot.bind(3, string, -1, SQLITE_STATIC);
        ++insroot;
        int noteId = insroot.lastInsertId();
        
        // Now insert an image record for each element of images.
        
        CSqliteStatement insimage(
            db,
            "INSERT INTO note_image                               \
                (note_id, note_offset, original_filename, image) \
            VALUES (?,?,?,?)"
        );
        insimage.bind(1, noteId);
        for (int i =0; i < images.size(); i++) {
            insimage.bind(2, images[i].s_offset);
            insimage.bind(3, images[i].s_filename.c_str(), -1, SQLITE_STATIC);
            auto img = readImage(images[i].s_filename);
            size_t      isize = img.first;
            void* pData = img.second;
            insimage.bind(4, pData, isize, SQLITE_STATIC);
            ++insimage;
            free(pData);
            insimage.reset();
        }
        pResult = new LogBookNote(db, noteId);
    }
    catch (CSqliteException e) {
        t.scheduleRollback();
        LogBook::Exception::rethrowSqliteException(e, "Creating note");
    }
    catch (...) {
        t.scheduleRollback();
        throw;
    }
    
    return pResult;
}

/////////////////////////////////////////////////////////////
// Private members


/**
 * parseLink
 *   Parses a markdown image link.  These have the form:
 *
 *  ![text if not image capable viewer](image path).
 *
 *  For example this might look like:
 *
 *  ![Here's what the scope showed](/user/fox/Desktop/scope.jpg)
 *
 * We do the following:
 *   -   verify this is an image link (starts with ![).
 *   -   figure out what the text is in the [] -> s_text
 *   -   figure out what the text is in the () -> s_link
 *   -   figure out how long the whole damned thing is -> s_length
 *
 * @param image - a notebook image specification.  This applies to
 *                the string: m_textInfo.s_contents
 * @return LinkInfo - the parsed link
 * @throw LogBook::Exception - if the parsing fails in some way.
 */
LogBookNote::LinkInfo
LogBookNote::parseLink(const NoteImage& image)
{
    LinkInfo result;
    
    // Get the string from the link start -> end:
    
    std::string parseme = m_textInfo.s_contents.substr(image.s_noteOffset);
    if (parseme.substr(0, 2) != "![") {
        std::stringstream msg;
        msg << "The text purported to be an image link at: "
            << m_textInfo.s_contents.substr(image.s_noteOffset, 25)
            << " is not a valid markdown link";
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    result.s_length = 2;      // ![
    
    // Find the location of the ] so we can pull the link text out:
    
    
    parseme = parseme.substr(2);
    auto closeSqloc = parseme.find_first_of("]", 0);
    if (closeSqloc == std::string::npos) {
        std::stringstream msg;
        msg << "Cannot find the ] in the text for the link at "
            << m_textInfo.s_contents.substr(image.s_noteOffset, 25)
            << " not a valid markdown link";
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    result.s_length += closeSqloc;
    result.s_text    = parseme.substr(0, closeSqloc);
    
    // Next character must be a (
    
    parseme = parseme.substr(closeSqloc + 1);
    if (parseme.substr(0,1) != "(") {
        std::stringstream msg;
        msg << "Missing link  in image link at "
            << m_textInfo.s_contents.substr(image.s_noteOffset, 25)
            << " don't see the ( after the ]";
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    result.s_length++;
    
    // Locate the ) as above and pull that into the link field.
    
    parseme= parseme.substr(1);       // After the (
    result.s_length++;
    int closeParen = parseme.find_first_of(")", 0);
    if (closeParen == std::string::npos) {
        std::stringstream msg;
        msg  << "Can't find the ) in the link text  at"
            << m_textInfo.s_contents.substr(image.s_noteOffset, 25)
            << " invalid markdown image link";
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    result.s_link = parseme.substr(0, closeParen);
    result.s_length += closeParen+1;
    
    return result;
}
/**
 * exportImage
 *    Creates a probably unique filename for the image
 *    in the temporary directory and writes the image file into that
 *    directory.   The filename is composed as follows:
 *
 *    - extract the basename from the original filename.
 *    - prepend to that tempdir/noteid_imageid_  where
 *      noteid is the text "note" followed by the primary key of the note
 *      and imageid is the text "image" followed by the primary key of the
 *      image.  Prepending is used to make the file extension/type
 *      easy to keep.
 *  @param image - references the image we're going to write.
 *  @return std::string - the path to the file we wrote.
 */
std::string
LogBookNote::exportImage(const NoteImage& image)
{
    // Ensure the directory exists:
    
    int stat = mkdir(LogBook::m_tempdir.c_str(), 0755);
    if (stat && (errno != EEXIST)) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to create temp file directory " << LogBook::m_tempdir
            << " : " << strerror(e);
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    
    //
    std::string result;          // filename we generate.
    
    char originalFile[image.s_originalFilename.size() + 1];   // Hope c++ allows this.
    strcpy(originalFile, image.s_originalFilename.c_str());
    char *name = basename(originalFile);
    // Now build up the full file path into result; assumes a unix pathsep.
    
    std::stringstream filename;
    filename << LogBook::m_tempdir << "/note" << image.s_noteId
        << "_image" << image.s_id << "_" << name;
    result = filename.str();
    
    // Now open the file; write the data and close the file:
    
    try {
        std::ofstream o(result, std::ios::binary | std::ios::out | std::ios::trunc);
        if (o.fail()) {
            std::stringstream msg;
            msg << "Failed to create image cache file: " << result;
            std::string m(msg.str());
            throw LogBook::Exception(m);
        }
        o.write(static_cast<const char*>(image.s_pImageData), image.s_imageLength);
        
        // Destruction closes the file.
    }
    catch (std::exception& e) {
        std::stringstream msg;
        msg << "Unable to open/write image temp file : " << result << " : "
            << e.what();
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    catch (...) {
        std::stringstream msg;
        msg << "Unable to open/write image temp file: " << result;
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    
    return result;
}
/**
 * editLink
 *    Given old link information and the new image filename
 *    returns a new markdown link that can be pasted into the
 *    document to replace the old one.
 *
 * @param link - link information from paresLink
 * @param filename - Filename the image will be stored in.
 * @return std::string full link.
 */
std::string
LogBookNote::editLink(const LinkInfo& link, const std::string& filename)
{
    std::string result;
    std::stringstream s;
    s << "![" << link.s_text << "](" << filename << ")";
    
    result = s.str();
    return result;
}

/**
 * CopyIn
 *    Replace the member data of this object with a deep copy
 *    of the member data of another object.
 */
void
LogBookNote::CopyIn(const LogBookNote& rhs)
{
    m_textInfo = rhs.m_textInfo;
    m_imageInfo = rhs.m_imageInfo;
}
/**
 * readImage(static)
 *    Sucks image data from file into dynamic memory.
 * @param filename = name of the file.
 * @return std::pair<size_t, void*>  - first is the size, second is a
 *     pointer to malloc'd data that contains the contents of the file.
 * @throw LogBook::Exception to report errors.
 */
std::pair<size_t, void*>
LogBookNote::readImage(const std::string& filename)
{
    std::pair<size_t, void*> result = {0, nullptr};
    
    // Get file information (incidently tests filename existence).
    // I believe that stat will do all the permission/existence checks
    // we need.
    
    struct stat fileInfo;
    int status = stat(filename.c_str(), &fileInfo);
    if (status) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to read image file " << filename
            << " creating note : " << strerror(e);
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    size_t nBytes = fileInfo.st_size;
    result.first  = nBytes;
    result.second = malloc(nBytes);     // Buffer for the fileblob
    if (!result.second) {               // allocation failed:
        int e = errno;
        std::stringstream msg;
        msg << "Failed to allocate storage for image file " << filename
            << " : " << strerror(e);
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    // here on in needs to be in try/catch just to allow the
    // storage to be freed:
    try {
        
        std::ifstream image(filename.c_str(), std::ios::in | std::ios::binary);
        if (image.fail())  {
            std::stringstream msg;
            msg << " Failed to open image file : " << filename
                << "while incorporating image into a note";
            std::string m(msg.str());
            throw LogBook::Exception(m);
        }
        
        image.read(static_cast<char*>(result.second), result.first);
        if (image.fail()) {
            std::stringstream msg;
            msg << "Failed to read image file " << filename
                << " whlie creating note";
            std::string m(msg.str());
            throw LogBook::Exception(m);
        }
    }
    catch (...) {
        free(result.second);
        throw;
    }
    return result;
}