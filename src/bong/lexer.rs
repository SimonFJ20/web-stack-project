use std::{iter::Peekable, str::Chars};

pub enum TokenType {
    SinglelineWhitespace,
    MultilineWhitespace,
    SinglelineComment,
    MultilineComment,

    Name,
    Id,
    Class,

    Int,
    Float,
    String,
    Null,
    False,
    True,

    LBrace,
    RBrace,
    LBracket,
    RBracket,

    Equal,
    Colon,
    SemiColon,
    Comma,
}

pub struct Token<'a> {
    token_type: TokenType,
    value: &'a str,
    line: u32,
    col: u32,
}

pub struct LexerError {
    line: u32,
    col: u32,
    message: String,
}

impl LexerError {
    pub fn new(line: u32, col: u32, message: String) -> Self {
        Self { line, col, message }
    }
}

pub struct Lexer<'a> {
    phantom: std::marker::PhantomData<&'a u32>,
}

impl<'a> Lexer<'a> {
    pub fn new(text: &'a str) -> Self {
        Self {
            phantom: std::marker::PhantomData {},
        }
    }
}

impl<'a> Iterator for Lexer<'a> {
    type Item = Result<Token<'a>, String>;

    fn next(&mut self) -> Option<Self::Item> {
        todo!()
    }
}
