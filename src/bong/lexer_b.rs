use std::str::Chars;

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Location {
    pub index: usize,
    pub line: isize,
    pub col: isize,
}

#[derive(Debug, PartialEq)]
pub enum TokenType {
    Name(String),
    Id(String),
    Class(String),
    SlWhitespace,
    MlWhitespace,
    SlComment,
    MlComment,
    Int(i64),
    Float(f64),
    String(String),
    Null,
    False,
    True,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
}

#[derive(Debug, PartialEq)]
pub struct Token {
    pub token_type: TokenType,
    pub location: Location,
    pub length: usize,
}

impl Token {
    pub fn value<'a>(&self, text: &'a str) -> &'a str {
        &text[self.location.index..self.location.index + self.length]
    }
}

#[derive(Debug, PartialEq)]
pub enum LexerErrorType {
    UnexpectedEof,
    UnexpectedChar(char),
    MalformedComment,
    UnterminatedString,
    InvalidInt(String),
    InvalidFloat(String),
}

#[derive(Debug, PartialEq)]
pub struct LexerError {
    pub error: LexerErrorType,
    pub location: Location,
}

pub struct Lexer<'a> {
    chars: Chars<'a>,
    current_char: Option<char>,
    index: usize,
    line: isize,
    col: isize,
}

impl<'a> Lexer<'a> {
    pub fn new(text: &'a str) -> Self {
        let mut lexer = Self {
            chars: text.chars(),
            current_char: None,
            index: 0,
            line: 1,
            col: 1,
        };
        lexer.current_char = lexer.chars.next();
        lexer
    }

    fn make_token(&mut self) -> Result<Token, LexerError> {
        match self.current() {
            Some(' ' | '\t' | '\n' | '\r') => self.make_whitespace(),
            Some('/') => self.make_comment(),
            Some('0'..='9') => self.make_number(),
            Some('"') => self.make_string(),
            Some('a'..='z' | 'A'..='Z' | '_') => self.make_name(),
            Some('#') => self.make_id(),
            Some('.') => self.make_class(),
            Some(c @ ('{' | '}' | '[' | ']')) => self.make_punctation(c),
            Some(c) => Err(self.error(LexerErrorType::UnexpectedChar(c))),
            _ => Err(self.error(LexerErrorType::UnexpectedEof)),
        }
    }

    fn make_whitespace(&mut self) -> Result<Token, LexerError> {
        let begin = self.location();
        self.step();
        loop {
            match self.current() {
                Some('\n') => {
                    self.step();
                    break loop {
                        match self.current() {
                            Some(' ' | '\t' | '\n' | '\r') => {
                                self.step();
                            }
                            _ => break Ok(self.token(TokenType::MlWhitespace, begin)),
                        }
                    };
                }
                Some(' ' | '\t' | '\r') => {
                    self.step();
                }
                _ => break Ok(self.token(TokenType::SlWhitespace, begin)),
            }
        }
    }

    fn make_comment(&mut self) -> Result<Token, LexerError> {
        let begin = self.location();
        self.step();
        match self.current() {
            Some('*') => self.make_mlcomment(begin),
            Some('/') => self.make_slcomment(begin),
            _ => Err(self.error(LexerErrorType::MalformedComment)),
        }
    }

    fn make_mlcomment(&mut self, begin: Location) -> Result<Token, LexerError> {
        self.step();
        let mut last = self
            .current()
            .ok_or(self.error(LexerErrorType::MalformedComment))?
            .clone();
        let mut depth = 1;
        loop {
            match (last, self.current()) {
                ('/', Some('*')) => {
                    depth += 1;
                    self.step();
                }
                ('*', Some('/')) => {
                    depth -= 1;
                    self.step();
                    if depth == 0 {
                        break Ok(self.token(TokenType::MlComment, begin));
                    }
                }
                (_, Some(c)) => {
                    last = c;
                    self.step();
                }
                _ => break Err(self.error(LexerErrorType::MalformedComment)),
            }
        }
    }

    fn make_slcomment(&mut self, begin: Location) -> Result<Token, LexerError> {
        self.step();
        loop {
            match self.current() {
                None | Some('\n') => break Ok(self.token(TokenType::SlComment, begin)),
                _ => {
                    self.step();
                }
            }
        }
    }

    fn make_number(&mut self) -> Result<Token, LexerError> {
        let begin = self.location();
        let mut value = String::new();
        value.push(self.current().unwrap());
        self.step();
        loop {
            match self.current() {
                Some(c @ '0'..='9') => {
                    value.push(c);
                    self.step();
                }
                Some(c @ '.') => {
                    value.push(c);
                    self.step();
                    break loop {
                        match self.current() {
                            Some(c @ '0'..='9') => {
                                value.push(c);
                                self.step();
                            }
                            _ => {
                                break Ok(self.token(
                                    TokenType::Float(value.parse::<f64>().map_err(|_| {
                                        self.error(LexerErrorType::InvalidFloat(value))
                                    })?),
                                    begin,
                                ))
                            }
                        }
                    };
                }
                _ => {
                    break Ok(self.token(
                        TokenType::Int(
                            value
                                .parse::<i64>()
                                .map_err(|_| self.error(LexerErrorType::InvalidInt(value)))?,
                        ),
                        begin,
                    ))
                }
            }
        }
    }

    fn make_string(&mut self) -> Result<Token, LexerError> {
        let begin = self.location();
        self.step();

        let mut value = String::new();
        loop {
            match self.current() {
                Some('"') => {
                    self.step();
                    break Ok(self.token(TokenType::String(value), begin));
                }
                Some('\\') => {
                    self.step();
                    match self.current() {
                        Some('t') => {
                            value.push('\t');
                            self.step();
                        }
                        Some('n') => {
                            value.push('\n');
                            self.step();
                        }
                        Some('r') => {
                            value.push('\r');
                            self.step();
                        }
                        Some(c) => {
                            value.push(c);
                            self.step();
                        }
                        _ => break Err(self.error(LexerErrorType::UnterminatedString)),
                    }
                }
                Some(c) => {
                    value.push(c);
                    self.step();
                }
                _ => break Err(self.error(LexerErrorType::UnterminatedString)),
            }
        }
    }

    fn make_name(&mut self) -> Result<Token, LexerError> {
        let begin = self.location();
        let value = self.make_identifier();
        Ok(self.token(
            match value.as_str() {
                "null" => TokenType::Null,
                "false" => TokenType::False,
                "true" => TokenType::True,
                _ => TokenType::Name(value),
            },
            begin,
        ))
    }

    fn make_id(&mut self) -> Result<Token, LexerError> {
        let begin = self.location();
        self.step();
        let value = self.make_identifier();
        Ok(self.token(TokenType::Id(value), begin))
    }

    fn make_class(&mut self) -> Result<Token, LexerError> {
        let begin = self.location();
        self.step();
        let value = self.make_identifier();
        Ok(self.token(TokenType::Class(value), begin))
    }

    fn make_identifier(&mut self) -> String {
        let mut value = String::new();
        value.push(self.current().unwrap());
        self.step();
        loop {
            match self.current() {
                Some(c @ ('0'..='9' | 'a'..='z' | 'A'..='Z' | '_')) => {
                    value.push(c);
                    self.step();
                }
                _ => break value,
            }
        }
    }

    fn make_punctation(&mut self, c: char) -> Result<Token, LexerError> {
        let begin = self.location();
        self.step();
        match c {
            '{' => Ok(self.token(TokenType::LBrace, begin)),
            '}' => Ok(self.token(TokenType::RBrace, begin)),
            '[' => Ok(self.token(TokenType::LBracket, begin)),
            ']' => Ok(self.token(TokenType::RBracket, begin)),
            _ => panic!("previous predicate should apply"),
        }
    }

    pub fn done(&mut self) -> bool {
        self.current_char.is_none()
    }
    fn step(&mut self) {
        self.index += 1;
        self.current_char = self.chars.next();
        match self.current_char {
            Some('\n') => {
                self.line += 1;
                self.col = 1;
            }
            Some(_) => self.col += 1,
            _ => {}
        }
    }
    fn current(&mut self) -> Option<char> {
        self.current_char
    }
    fn location(&self) -> Location {
        Location {
            index: self.index,
            line: self.line,
            col: self.col,
        }
    }
    fn token(&self, token_type: TokenType, begin: Location) -> Token {
        Token {
            token_type,
            location: begin,
            length: self.index - begin.index,
        }
    }
    fn error(&self, error_type: LexerErrorType) -> LexerError {
        LexerError {
            error: error_type,
            location: self.location(),
        }
    }
}

impl<'a> Iterator for Lexer<'a> {
    type Item = Result<Token, LexerError>;

    fn next(&mut self) -> Option<Self::Item> {
        if !self.done() {
            Some(self.make_token())
        } else {
            None
        }
    }
}

pub fn lex(text: &str) -> Result<Vec<Token>, LexerError> {
    Lexer::new(text).collect()
}
