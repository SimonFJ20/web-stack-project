#![allow(dead_code)]

use super::lexer::Token;
use std::collections::HashMap;

pub enum Node {
    Element {
        name: String,
        ids: Vec<String>,
        classes: Vec<String>,
        properties: HashMap<String, Box<Node>>,
        values: Vec<Node>,
    },
    Object(HashMap<String, Box<Node>>),
    Array(Vec<Node>),
    Int(i64),
    Float(f64),
    String(String),
    Bool(bool),
    Null,
}

pub struct Parser {
    tokens: Vec<Token>,
    index: usize,
}

type ParserError = String;

enum ElementField {
    Id(String),
    Class(String),
    Property(Box<Node>),
    Value(Node),
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Self { tokens, index: 0 }
    }

    pub fn parse_top_level(&mut self) -> Result<Node, ParserError> {
        match self.current() {
            Some(Token::Name(_)) => self.parse_element(),
            Some(_) => self.parse_value(),
            None => Err("expected value or element".to_owned()),
        }
    }

    fn parse_element(&mut self) -> Result<Node, ParserError> {
        let name = match self.current() {
            Some(Token::Name(value)) => value.clone(),
            _ => panic!("checked by previous predicate"),
        };
        self.step();
        todo!()
    }

    fn parse_singe_line_fields(&mut self) -> Result<Vec<ElementField>, ParserError> {
        let mut fields = Vec::<ElementField>::new();
        match self.current() {
            Some(Token::Id(value)) => {
                fields.push(ElementField::Id(value.clone()));
                self.step();
                todo!()
            }
            Some(Token::Class(value)) => {
                fields.push(ElementField::Class(value.clone()));
                self.step();
                todo!()
            }
            Some(_) => todo!(),
            _ => Ok(fields),
        }
    }

    fn parse_value(&mut self) -> Result<Node, ParserError> {
        match self.current() {
            Some(Token::LBrace(_)) => self.parse_object(),
            Some(Token::LBracket(_)) => self.parse_object(),
            Some(Token::Int(value)) => {
                let value = value.parse().map_err(|_| "malformed int".to_string())?;
                self.step();
                Ok(Node::Int(value))
            }
            Some(Token::Float(value)) => {
                let value = value.parse().map_err(|_| "malformed float".to_string())?;
                self.step();
                Ok(Node::Float(value))
            }
            Some(Token::String(value)) => {
                let value = value[1..value.len() - 1].to_string();
                self.step();
                Ok(Node::String(value))
            }
            Some(Token::False(_)) => {
                self.step();
                Ok(Node::Bool(false))
            }
            Some(Token::True(_)) => {
                self.step();
                Ok(Node::Bool(false))
            }
            Some(Token::Null(_)) => {
                self.step();
                Ok(Node::Null)
            }
            Some(_) => Err("unexpected token, expected value".to_owned()),
            None => Err("expected value".to_owned()),
        }
    }

    fn parse_object(&mut self) -> Result<Node, ParserError> {
        self.step();
        // object_properties -> (_ (...)):? _ <=> object_properties -> _ | _ (...) _
        self.parse_optional_whitespace()?;
        let mut values = HashMap::<String, Box<Node>>::new();
        match self.current() {
            Some(Token::RBrace(_)) => {
                self.step();
                Ok(Node::Object(values))
            }
            Some(t @ (Token::Name(_) | Token::String(_))) => {
                let key = match t {
                    Token::Name(v) => v.clone(),
                    Token::String(v) => v[1..v.len() - 1].to_string(),
                    _ => panic!("checked by previous predicate"),
                };
                self.step();
                self.parse_optional_whitespace()?;
                match self.current() {
                    Some(Token::Equal(_) | Token::Colon(_)) => {}
                    _ => return Err("expected ':' or '='".to_string()),
                }
                self.step();
                self.parse_optional_whitespace()?;
                values.insert(key, Box::new(self.parse_value()?));
                self.parse_object_tail(values)
            }
            _ => Err("expected Name, String or '}'".to_string()),
        }
    }

    fn parse_object_tail(
        &mut self,
        mut values: HashMap<String, Box<Node>>,
    ) -> Result<Node, String> {
        loop {
            self.parse_optional_whitespace()?;
            match self.current() {
                Some(Token::RBrace(_)) => {
                    self.step();
                    break Ok(Node::Object(values));
                }
                Some(Token::Comma(_)) => {
                    self.step();
                    self.parse_optional_whitespace()?;
                    match self.current() {
                        Some(Token::RBrace(_)) => {
                            self.step();
                            break Ok(Node::Object(values));
                        }
                        Some(t @ (Token::Name(_) | Token::String(_))) => {
                            let key = match t {
                                Token::Name(v) => v.clone(),
                                Token::String(v) => v[1..v.len() - 1].to_string(),
                                _ => panic!("unterminated object, checked by previous predicate"),
                            };
                            self.step();
                            self.parse_optional_whitespace()?;
                            match self.current() {
                                Some(Token::Equal(_) | Token::Colon(_)) => {}
                                _ => return Err("expected ':' or '='".to_string()),
                            }
                            self.step();
                            self.parse_optional_whitespace()?;
                            values.insert(key, Box::new(self.parse_value()?));
                            self.parse_optional_whitespace()?;
                        }
                        _ => {
                            break Err(
                                "unterminated object, expected Name, String or '}'".to_string()
                            )
                        }
                    }
                }
                _ => break Err("unterminated object, expected ',' or '}'".to_string()),
            }
        }
    }

    fn parse_array(&mut self) -> Result<Node, ParserError> {
        self.step();
        self.parse_optional_whitespace()?;
        let mut values = Vec::<Node>::new();
        match self.current() {
            Some(Token::RBracket(_)) => {
                self.step();
                Ok(Node::Array(values))
            }
            Some(_) => {
                values.push(self.parse_value()?);
                loop {
                    match self.current() {
                        Some(Token::RBracket(_)) => {
                            self.step();
                            break Ok(Node::Array(values));
                        }
                        Some(Token::Comma(_)) => {
                            self.step();
                            self.parse_optional_whitespace()?;
                            match self.current() {
                                Some(Token::RBracket(_)) => {
                                    self.step();
                                    self.parse_optional_whitespace()?;
                                    break Ok(Node::Array(values));
                                }
                                _ => {
                                    self.step();
                                    self.parse_optional_whitespace()?;
                                    values.push(self.parse_value()?)
                                }
                            }
                        }
                        _ => break Err("unterminated array, expected Value or ']'".to_string()),
                    }
                }
            }
            _ => Err("unterminated array, expected Value or ']'".to_string()),
        }
    }

    fn parse_mandatory_linebreak(&mut self) -> Result<(), ParserError> {
        self.parse_single_line_optional_whitespace()?;
        match self.current() {
            Some(Token::MlWhitespace(_) | Token::MlComment(_) | Token::SemiColon(_)) => {
                self.step();
                loop {
                    match self.current() {
                        Some(
                            Token::MlWhitespace(_)
                            | Token::SlWhitespace(_)
                            | Token::MlComment(_)
                            | Token::SlComment(_)
                            | Token::SemiColon(_),
                        ) => {
                            self.step();
                        }
                        _ => break Ok(()),
                    }
                }
            }
            _ => Err("expected linebreak".to_string()),
        }
    }

    fn parse_single_line_mandatory_whitespace(&mut self) -> Result<(), ParserError> {
        match self.current() {
            Some(Token::SlWhitespace(_) | Token::SlComment(_)) => {
                self.step();
                self.parse_single_line_optional_whitespace()
            }
            _ => Err("expected whitespace".to_string()),
        }
    }

    fn parse_single_line_optional_whitespace(&mut self) -> Result<(), ParserError> {
        loop {
            match self.current() {
                Some(Token::SlWhitespace(_) | Token::SlComment(_)) => {
                    self.step();
                }
                _ => break Ok(()),
            }
        }
    }

    fn parse_mandatory_whitespace(&mut self) -> Result<(), ParserError> {
        match self.current() {
            Some(
                Token::MlWhitespace(_)
                | Token::SlWhitespace(_)
                | Token::MlComment(_)
                | Token::SlComment(_),
            ) => {
                self.step();
                self.parse_optional_whitespace()
            }
            _ => Err("expected whitespace".to_string()),
        }
    }

    fn parse_optional_whitespace(&mut self) -> Result<(), ParserError> {
        loop {
            match self.current() {
                Some(
                    Token::MlWhitespace(_)
                    | Token::SlWhitespace(_)
                    | Token::MlComment(_)
                    | Token::SlComment(_),
                ) => {
                    self.step();
                }
                _ => break Ok(()),
            }
        }
    }

    fn step(&mut self) {
        self.index += 1
    }
    fn current(&self) -> Option<&Token> {
        self.tokens.get(self.index)
    }
}
