#pragma once

namespace scriptlang {

enum class Errors {
    NotImplemented,
    LexerNoTokenYet,
    LexerStringNotTerminated,
    LexerUnexpectedCharacer,
    LexerMultilineCommentNotTerminated,
    NoLexerOutput,
    ParserExhausted,
    ParserMalformed,
    ParserUnexpected,
};

}
