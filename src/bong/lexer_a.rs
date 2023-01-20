#[derive(Debug, Clone, PartialEq)]
pub enum LexerErrorType {
    UnexpectedToken(char),
    InvalidConstructor,
}

#[derive(Debug, Clone, PartialEq)]
pub struct LexerError {
    error: LexerErrorType,
    line: isize,
    col: isize,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Token {
    Name(String),
    Id(String), // not implemented
    Class(String),
    SlWhitespace(String),
    MlWhitespace(String),
    SlComment(String),
    MlComment(String), // not implemented
    Int(String),       // not implemented
    Float(String),     // not implemented
    String(String),
    Null(String),  // not implemented
    True(String),  // not implemented
    False(String), // not implemented
    LBrace(String),
    RBrace(String),
    LBracket(String), // not implemented
    RBracket(String), // not implemented
}

enum Mode {
    Name,
    Class,
    String,
    EscapedString,
    SlWhitespace,
    MlWhitespace,
    SlComment,
}

impl Mode {
    fn token_constructor(&self) -> Result<Box<dyn Fn(String) -> Token>, LexerErrorType> {
        match self {
            Mode::Name => Ok(Box::new(Token::Name)),
            Mode::Class => Ok(Box::new(Token::Class)),
            Mode::String => Ok(Box::new(Token::String)),
            Mode::SlWhitespace => Ok(Box::new(Token::SlWhitespace)),
            Mode::MlWhitespace => Ok(Box::new(Token::MlWhitespace)),
            Mode::SlComment => Ok(Box::new(Token::SlComment)),
            Mode::EscapedString => Err(LexerErrorType::InvalidConstructor),
        }
    }
}

pub fn lex(code: String) -> Result<Vec<Token>, LexerError> {
    let mut tokens = Vec::new();
    let mut value = Vec::new();
    let mut iter = code.chars();
    let mut mode = Mode::SlWhitespace;
    let mut line = 0;
    let mut col = 0;
    let position_map = move |error: LexerErrorType| LexerError { error, line, col };
    loop {
        let c = iter.next();
        if c.is_none() {
            break;
        };
        match c.unwrap() {
            '.' => {
                match mode {
                    m @ Mode::Name
                    | m @ Mode::Class
                    | m @ Mode::SlWhitespace
                    | m @ Mode::MlWhitespace => {
                        let string_value = value.iter().collect();
                        let constructor = m.token_constructor().map_err(position_map)?;
                        tokens.push(constructor(string_value));
                        value.clear();
                        mode = Mode::Class;
                    }
                    Mode::String | Mode::SlComment => {}
                    Mode::EscapedString => {
                        return Err(LexerError {
                            line,
                            col,
                            error: LexerErrorType::UnexpectedToken('.'),
                        })
                    }
                };
                value.push('.');
            }
            '\\' => match mode {
                Mode::String => {
                    value.push('\\');
                    mode = Mode::EscapedString;
                }
                _ => {
                    return Err(LexerError {
                        line,
                        col,
                        error: LexerErrorType::UnexpectedToken('\\'),
                    })
                }
            },
            '"' => {
                match mode {
                    m @ Mode::String => {
                        mode = Mode::SlWhitespace;
                        value.push('"');
                        let string_value = value.iter().collect();
                        let constructor = m.token_constructor().map_err(position_map)?;
                        tokens.push(constructor(string_value));
                        value.clear();
                    }
                    m @ Mode::SlWhitespace | m @ Mode::MlWhitespace => {
                        mode = Mode::String;
                        let string_value = value.iter().collect();
                        let constructor = m.token_constructor().map_err(position_map)?;
                        tokens.push(constructor(string_value));
                        value.clear();
                        value.push('"');
                    }
                    Mode::EscapedString => {
                        value.push('"');
                        mode = Mode::String;
                    }
                    Mode::SlComment => {
                        value.push('"');
                    }
                    _ => {
                        return Err(LexerError {
                            line,
                            col,
                            error: LexerErrorType::UnexpectedToken('"'),
                        })
                    }
                };
            }
            '{' => match mode {
                m @ Mode::Name
                | m @ Mode::Class
                | m @ Mode::MlWhitespace
                | m @ Mode::SlWhitespace => {
                    let string_value = value.iter().collect();
                    let constructor = m.token_constructor().map_err(position_map)?;
                    tokens.push(constructor(string_value));
                    value.clear();
                    mode = Mode::SlWhitespace;
                    tokens.push(Token::LBrace(String::from('{')));
                }
                Mode::EscapedString => {
                    return Err(LexerError {
                        line,
                        col,
                        error: LexerErrorType::UnexpectedToken('{'),
                    })
                }
                Mode::String | Mode::SlComment => {
                    value.push('{');
                }
            },
            '}' => match mode {
                m @ Mode::Name
                | m @ Mode::Class
                | m @ Mode::MlWhitespace
                | m @ Mode::SlWhitespace => {
                    let string_value = value.iter().collect();
                    let constructor = m.token_constructor().map_err(position_map)?;
                    tokens.push(constructor(string_value));
                    value.clear();
                    mode = Mode::SlWhitespace;
                    tokens.push(Token::RBrace(String::from('}')));
                }
                Mode::String | Mode::SlComment => {
                    value.push('}');
                }
                Mode::EscapedString => {
                    return Err(LexerError {
                        line,
                        col,
                        error: LexerErrorType::UnexpectedToken('}'),
                    })
                }
            },
            c @ ' ' | c @ '\r' => {
                match mode {
                    m @ Mode::Name | m @ Mode::Class => {
                        let string_value = value.iter().collect();
                        let constructor = m.token_constructor().map_err(position_map)?;
                        tokens.push(constructor(string_value));
                        value.clear();
                        mode = Mode::SlWhitespace;
                    }
                    Mode::String | Mode::SlComment | Mode::MlWhitespace | Mode::SlWhitespace => {}
                    Mode::EscapedString => {
                        return Err(LexerError {
                            line,
                            col,
                            error: LexerErrorType::UnexpectedToken(c),
                        })
                    }
                };
                value.push(c);
            }
            c @ '\n' => {
                match mode {
                    m @ Mode::Name | m @ Mode::Class | m @ Mode::SlComment => {
                        let string_value = value.iter().collect();
                        let constructor = m.token_constructor().map_err(position_map)?;
                        tokens.push(constructor(string_value));
                        value.clear();
                        mode = Mode::MlWhitespace;
                    }
                    Mode::MlWhitespace | Mode::SlWhitespace => {
                        mode = Mode::MlWhitespace;
                    }
                    Mode::String => {}
                    Mode::EscapedString => {
                        return Err(LexerError {
                            line,
                            col,
                            error: LexerErrorType::UnexpectedToken('\n'),
                        })
                    }
                };
                value.push(c);
                line += 1;
                col = -1;
            }
            '/' => {
                match mode {
                    Mode::String | Mode::SlComment => {}
                    m @ Mode::Name
                    | m @ Mode::Class
                    | m @ Mode::SlWhitespace
                    | m @ Mode::MlWhitespace => {
                        let string_value = value.iter().collect();
                        let constructor = m.token_constructor().map_err(position_map)?;
                        tokens.push(constructor(string_value));
                        value.clear();
                        mode = Mode::SlComment;
                    }
                    Mode::EscapedString => {
                        return Err(LexerError {
                            line,
                            col,
                            error: LexerErrorType::UnexpectedToken('/'),
                        })
                    }
                };
                value.push('/');
            }
            v @ 'A'..='Z' | v @ 'a'..='z' | v @ '0'..='9' => {
                match mode {
                    Mode::String | Mode::SlComment | Mode::Name | Mode::Class => {}
                    m @ Mode::SlWhitespace | m @ Mode::MlWhitespace => {
                        let string_value = value.iter().collect();
                        let constructor = m.token_constructor().map_err(position_map)?;
                        tokens.push(constructor(string_value));
                        value.clear();
                        mode = Mode::Name;
                    }

                    Mode::EscapedString => {
                        return Err(LexerError {
                            line,
                            col,
                            error: LexerErrorType::UnexpectedToken(v),
                        })
                    }
                };
                value.push(v);
            }
            unrecognized_char => {
                return Err(LexerError {
                    line,
                    col,
                    error: LexerErrorType::UnexpectedToken(unrecognized_char),
                })
            }
        }
        col += 1;
    }

    Ok(tokens)
}
