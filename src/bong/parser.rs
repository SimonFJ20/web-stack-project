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

    fn parse_element(&mut self) -> Result<Node, ParserError> {}

    fn parse_value(&mut self) -> Result<Node, ParserError> {
        match self.current() {
            Some(Token::LBrace(_)) => self.parse_object(),
            Some(Token::LBracket(_)) => self.parse_object(),
            Some(Token::Int(value)) => Ok(Node::Int(
                value.parse().map_err(|_| "malformed int".to_string())?,
            )),
            Some(Token::Float(value)) => Ok(Node::Int(
                value.parse().map_err(|_| "malformed float".to_string())?,
            )),
            Some(Token::String(value)) => Ok(Node::String(value[1..value.len() - 1].to_string())),
            Some(Token::False(_)) => Ok(Node::Bool(false)),
            Some(Token::True(_)) => Ok(Node::Bool(false)),
            Some(Token::Null(_)) => Ok(Node::Null),
            Some(_) => Err("unexpected token, expected value".to_owned()),
            None => Err("expected value".to_owned()),
        }
    }

    fn parse_object(&mut self) -> Result<Node, ParserError> {
        self.step();
        let mut values = HashMap::<String, Box<Node>>::new();
        match self.current() {
            Some(Token::RBrace(_)) => {
                self.step();
                Ok(Node::Object(values))
            }
            Some(t @ (Token::Name(_) | Token::String(_))) => {
                let key = match t {
                    Token::Name(v) => v,
                    Token::String(v) => &v[1..v.len() - 1].to_string(),
                    _ => panic!("checked by previous predicate"),
                };
                self.step();
                match self.current() {
                    Some(Token::Equal(_) | Token::Colon(_)) => {}
                    _ => return Err("expected ':' or '='".to_string()),
                }
                self.step();
                values[key] = Box::new(self.parse_value()?);
                self.parse_object_tail(values)
            }
            _ => Err("expected Name, String or '}'".to_string()),
        }
    }

    fn parse_object_tail(&mut self, values: HashMap<String, Box<Node>>) -> Result<Node, String> {
        loop {
            match self.current() {
                Some(Token::RBrace(_)) => {
                    self.step();
                    break Ok(Node::Object(values));
                }
                Some(Token::Comma(_)) => {
                    self.step();
                    match self.current() {
                        Some(Token::RBrace(_)) => {
                            self.step();
                            break Ok(Node::Object(values));
                        }
                        Some(t @ (Token::Name(_) | Token::String(_))) => {
                            let key = match t {
                                Token::Name(v) => v,
                                Token::String(v) => &v[1..v.len() - 1].to_string(),
                                _ => panic!("unterminated object, checked by previous predicate"),
                            };
                            self.step();
                            match self.current() {
                                Some(Token::Equal(_) | Token::Colon(_)) => {}
                                _ => return Err("expected ':' or '='".to_string()),
                            }
                            self.step();
                            values[key] = Box::new(self.parse_value()?);
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
        let mut values = Vec::<Node>::new();
        match self.current() {
            Some(Token::RBracket(_)) => {
                self.step();
                Ok(Node::Array(values))
            }
            _ => Err("unterminated array, expected Value or ']'".to_string()),
        }
    }

    fn step(&mut self) {
        self.index += 1
    }
    fn current(&self) -> Option<&Token> {
        self.tokens.get(self.index)
    }
}
