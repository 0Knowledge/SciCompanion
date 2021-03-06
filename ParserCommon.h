#pragma once

// Common parser routines


// Uncomment this to get parsing debug spew.
//#define PARSE_DEBUG 1

//
// Syntax parser inspired by boost's Spirit (but much less flexible and more performant)
// 

#ifdef DEBUG
#define _DEBUG_PARSER(x) (x).SetName(#x);
#else
#define _DEBUG_PARSER(x)
#endif

#ifdef PARSE_DEBUG
extern int g_ParseIndent;
#endif

//
// A comment was detected - add it (text and endpoints) to the script.
//
template<typename TContext, typename _It>
inline void _DoComment(TContext *pContext, const _It &streamBegin, const _It &streamEnd)
{
    std::string comment;
    // Transfer to string:
    copy(streamBegin, streamEnd, back_inserter(comment));
    // Create a new Comment syntax node and add it to the script
    auto_ptr<sci::Comment> pComment(new sci::Comment());
    pComment->SetPosition(streamBegin.GetPosition());
    pComment->SetEndPosition(streamEnd.GetPosition());
    pComment->SetName(comment); // The comment text
    pContext->Script().AddComment(pComment.get());
    pComment.release();
}

template<typename TContext, typename _It>
inline void _EatWhitespaceAndComments(TContext *pContext, _It &stream)
{
    bool fDone = false;
    while (!fDone)
    {
        fDone = true;
        // Eat whitespace
        while (isspace(*stream))
        {
            fDone = false;
            ++stream;
        }
        _It streamSave(stream);
        if (*stream == '/')
        {
            char ch = *(++stream);
            if (ch == '/')
            {
                // Go until end of line
                while ((ch = *(++stream)) && (ch != '\n')) {} // Look for \n or EOF
                if (ch == '\n') // As opposed to EOF
                {
                    ++stream; // Move past \n
                }
                fDone = false; // Check for whitespace again

                // Comment gathering.  This may be expensive, so only do this if pContext is non-NULL
                if (pContext)
                {
                    _DoComment(pContext, streamSave, stream);
                }
            }
            else if (ch == '*')
            {
                // Go until */
                bool fLookingForSlash = false;
                while (TRUE)
                {
                    ch = *(++stream);
                    if (ch == 0)
                    {
                        break;
                    }
                    else if (ch == '*')
                    {
                        fLookingForSlash = true;
                    }
                    else if (fLookingForSlash)
                    {
                        if (ch == '/')
                        {
                            break; // We found a */
                        }
                        else
                        {
                            fLookingForSlash = false;
                        }
                    }
                }
                if (ch == '/') // As opposed to 0
                {
                    ++stream; // Move past '/'
                }
                if (pContext)
                {
                    _DoComment(pContext, streamSave, stream);
                }
                fDone = false; // Check for whitespace again
            }
            else
            {
                // We're done.
                stream = streamSave;
                fDone = true;
            }
        }
    }
}

//
// Optimized delimiter reader
//
template<typename _TContext, typename _It, char Q1, char Q2>
bool _ReadString(_It &stream, std::string &str)
{
    str.clear();
    while (Q1 == *stream)
    {
        char chPrev = 0;
        char ch;
        // Continue while no EOF, and while the character isn't the closing quote
        // (unless the previous character was an escape char)
        bool fEscape = false;
        while ((ch = *(++stream)) && ((ch != Q2) || (chPrev == '\\')))
        {
            chPrev = ch;
            if (!fEscape)
            {
                if (ch == '\\')
                {
                    // Escape char, don't add to strign.
                    fEscape = true;
                }
                else
                {
                    str += ch;
                }
            }
            else
            {
                // Last char was an escape char.
                fEscape = false;
                switch(ch)
                {
                case 'n':
                    str += '\n';
                    break;
                case 't':
                    str += '\t';
                    break;
                    // Any others we should handle?
                default:
                    str += ch;
                    break;
                }
            }
        }
        if (ch == Q2)
        {
            ++stream;
            if (*stream == '+')
            {
                // If we encountered a '+', then skip whitespace until we hit another open quote
                // "huwehuierger"+   "gejriger"
                ++stream;
                _EatWhitespaceAndComments<_TContext, _It>(NULL, stream);
                // ... and continue...
            }
            else
            {
                return true; // We're done
            }
        }
        else
        {
            ASSERT(ch == 0); // EOF
            return false;
        }
    }
    return false;
}