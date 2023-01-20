#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ErrorType {
    UnexpectedToken(char),
    InvalidConstructor,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Error {
    error: ErrorType,
    line: isize,
    col: isize,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Token {
    Name(String),
    Id(String),
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
    LBracket(String),
    RBracket(String),
}

#[derive(PartialEq)]
enum Mode {
    Name,
    Class,
    Id,
    String,
    EscapedString,
    SlWhitespace,
    MlWhitespace,
    SlComment,
}

impl Mode {
    fn token_constructor(&self) -> Result<Box<dyn Fn(String) -> Token>, ErrorType> {
        match self {
            Mode::Name => Ok(Box::new(Token::Name)),
            Mode::Class => Ok(Box::new(Token::Class)),
            Mode::String => Ok(Box::new(Token::String)),
            Mode::SlWhitespace => Ok(Box::new(Token::SlWhitespace)),
            Mode::MlWhitespace => Ok(Box::new(Token::MlWhitespace)),
            Mode::SlComment => Ok(Box::new(Token::SlComment)),
            Mode::Id => Ok(Box::new(Token::Id)),
            Mode::EscapedString => Err(ErrorType::InvalidConstructor),
        }
    }
}

fn collect_into_token_and_push(
    constructor: &dyn Fn(String) -> Token,
    tokens: &mut Vec<Token>,
    value: &mut Vec<char>,
) {
    let token = constructor(value.iter().collect());
    tokens.push(token);
    value.clear();
}

#[allow(dead_code)]
pub fn lexer(code_to_lex: &str) -> Result<Vec<Token>, Error> {
    let mut tokens = Vec::new();
    let mut value = Vec::new();
    let mut mode = Mode::SlWhitespace;
    let mut line = 0;
    let mut col = 0;
    let position_map = move |error: ErrorType| Error { error, line, col };
    for current_char in code_to_lex.chars() {
        match current_char {
            v @ ('.' | '#') => {
                match mode {
                    m @ (Mode::Name
                    | Mode::Class
                    | Mode::Id
                    | Mode::SlWhitespace
                    | Mode::MlWhitespace) => {
                        collect_into_token_and_push(
                            &m.token_constructor().map_err(position_map)?,
                            &mut tokens,
                            &mut value,
                        );
                        mode = match v {
                            '.' => Mode::Class,
                            '#' => Mode::Id,
                            _ => panic!("race condition"),
                        };
                    }
                    Mode::String | Mode::SlComment => {}
                    Mode::EscapedString => {
                        return Err(Error {
                            line,
                            col,
                            error: ErrorType::UnexpectedToken('.'),
                        })
                    }
                };
                value.push(v);
            }
            '\\' => match mode {
                Mode::String => {
                    value.push('\\');
                    mode = Mode::EscapedString;
                }
                _ => {
                    return Err(Error {
                        line,
                        col,
                        error: ErrorType::UnexpectedToken('\\'),
                    })
                }
            },
            '"' => {
                match mode {
                    m @ Mode::String => {
                        mode = Mode::SlWhitespace;
                        value.push('"');

                        collect_into_token_and_push(
                            &m.token_constructor().map_err(position_map)?,
                            &mut tokens,
                            &mut value,
                        );
                    }
                    m @ (Mode::SlWhitespace | Mode::MlWhitespace) => {
                        mode = Mode::String;
                        collect_into_token_and_push(
                            &m.token_constructor().map_err(position_map)?,
                            &mut tokens,
                            &mut value,
                        );
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
                        return Err(Error {
                            line,
                            col,
                            error: ErrorType::UnexpectedToken('"'),
                        })
                    }
                };
            }

            v @ ('{' | '}' | '[' | ']') => match mode {
                m @ (Mode::Name
                | Mode::Class
                | Mode::Id
                | Mode::MlWhitespace
                | Mode::SlWhitespace) => {
                    collect_into_token_and_push(
                        &m.token_constructor().map_err(position_map)?,
                        &mut tokens,
                        &mut value,
                    );
                    mode = Mode::SlWhitespace;
                    let constructor = match v {
                        '{' => Token::LBrace,
                        '}' => Token::RBrace,
                        '[' => Token::LBracket,
                        ']' => Token::RBracket,
                        _ => panic!("race condition"),
                    };
                    tokens.push(constructor(String::from(v)));
                }
                Mode::EscapedString => {
                    return Err(Error {
                        line,
                        col,
                        error: ErrorType::UnexpectedToken(v),
                    })
                }
                Mode::String | Mode::SlComment => {
                    value.push(v);
                }
            },
            c @ (' ' | '\r') => {
                match mode {
                    m @ (Mode::Name | Mode::Class | Mode::Id) => {
                        collect_into_token_and_push(
                            &m.token_constructor().map_err(position_map)?,
                            &mut tokens,
                            &mut value,
                        );
                        mode = Mode::SlWhitespace;
                    }
                    Mode::String | Mode::SlComment | Mode::MlWhitespace | Mode::SlWhitespace => {}
                    Mode::EscapedString => {
                        return Err(Error {
                            line,
                            col,
                            error: ErrorType::UnexpectedToken(c),
                        })
                    }
                };
                value.push(c);
            }
            c @ '\n' => {
                match mode {
                    m @ (Mode::Name | Mode::Class | Mode::Id | Mode::SlComment) => {
                        collect_into_token_and_push(
                            &m.token_constructor().map_err(position_map)?,
                            &mut tokens,
                            &mut value,
                        );
                        mode = Mode::MlWhitespace;
                    }
                    Mode::MlWhitespace | Mode::SlWhitespace => {
                        mode = Mode::MlWhitespace;
                    }
                    Mode::String => {}
                    Mode::EscapedString => {
                        return Err(Error {
                            line,
                            col,
                            error: ErrorType::UnexpectedToken('\n'),
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
                    m @ (Mode::Name
                    | Mode::Class
                    | Mode::Id
                    | Mode::SlWhitespace
                    | Mode::MlWhitespace) => {
                        collect_into_token_and_push(
                            &m.token_constructor().map_err(position_map)?,
                            &mut tokens,
                            &mut value,
                        );
                        mode = Mode::SlComment;
                    }
                    Mode::EscapedString => {
                        return Err(Error {
                            line,
                            col,
                            error: ErrorType::UnexpectedToken('/'),
                        })
                    }
                };
                value.push('/');
            }
            v @ ('A'..='Z' | 'a'..='z' | '0'..='9') => {
                match mode {
                    Mode::Name | Mode::Class | Mode::Id => {
                        if v.is_numeric()
                            && (value.is_empty() || mode == Mode::Id && value.len() == 1)
                        {
                            return Err(Error {
                                line,
                                col,
                                error: ErrorType::UnexpectedToken(v),
                            });
                        }
                    }
                    Mode::String | Mode::SlComment => {}
                    m @ (Mode::SlWhitespace | Mode::MlWhitespace) => {
                        collect_into_token_and_push(
                            &m.token_constructor().map_err(position_map)?,
                            &mut tokens,
                            &mut value,
                        );
                        mode = Mode::Name;
                    }
                    Mode::EscapedString => {
                        return Err(Error {
                            line,
                            col,
                            error: ErrorType::UnexpectedToken(v),
                        })
                    }
                };
                value.push(v);
            }
            unrecognized_char => match mode {
                Mode::String => {
                    value.push(unrecognized_char);
                }
                _ => {
                    return Err(Error {
                        line,
                        col,
                        error: ErrorType::UnexpectedToken(unrecognized_char),
                    });
                }
            },
        }
        col += 1;
    }

    Ok(tokens)
}

#[test]
fn test_example_1() {
    let text = "text.title {
    // text { \"hello world\" }
    \"hello world\"
}";
    let tokens = lexer(text);
    assert_eq!(
        tokens,
        Ok(vec![
            Token::SlWhitespace("".to_string()),
            Token::Name("text".to_string()),
            Token::Class(".title".to_string()),
            Token::SlWhitespace(" ".to_string()),
            Token::LBrace("{".to_string()),
            Token::MlWhitespace("\n    ".to_string()),
            Token::SlComment("// text { \"hello world\" }".to_string()),
            Token::MlWhitespace("\n    ".to_string()),
            Token::String("\"hello world\"".to_string()),
            Token::MlWhitespace("\n".to_string()),
            Token::RBrace("}".to_string()),
        ])
    )
}
