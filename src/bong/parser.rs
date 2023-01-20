use std::iter::Map;

use crate::bong::lexer::Token;

pub enum Node {
    Element {
        name: String,
        ids: Vec<String>,
        classes: Vec<String>,
        properties: Map<String, Box<Node>>,
        values: Vec<Node>,
    },
    Object(Map<String, Box<Node>>),
    Array(Vec<Node>),
    Int(i64),
    Float(f64),
    String(String),
    Bool(bool),
}

// pub struct Parser {
//     tokens: Vec<Token>,
// }

// impl Parser {
//     pub fn new(lexer: Lexer) -> Self {
//         Self { lexer }
//     }
// }

// type ParserError = String;

// impl Parser {
//     pub fn new(tokens: Vec<Token>) -> Self {
//         Self { tokens, index: 0 }
//     }

//     pub fn parse_top_level(&mut self) -> Result<Node, ParserError> {
//         match self.peek() {
//             Some(Token::Name(_)) => self.parse_element(),
//             Some(_) => self.parse_value(),
//             None => Err("expected value or element".to_owned()),
//         }
//     }

//     fn parse_element(&mut self) -> Result<Node, ParserError> {}

//     fn parse_value(&mut self) -> Result<Node, ParserError> {
//         match self.peek() {
//             Some(Token::LBrace(_)) => self.parse_object(),
//             Some(Token::LBracket(_)) => self.parse_object(),
//             Some(_) => Err("unexpected token, expected value".to_owned()),
//             None => Err("expected value".to_owned()),
//         }
//     }

//     fn parse_object(&mut self) -> Result<Node, ParserError> {}
//     fn parse_array(&mut self) -> Result<Node, ParserError> {}
//     fn parse_number(&mut self) -> Result<Node, ParserError> {}
//     fn parse_string(&mut self) -> Result<Node, ParserError> {}
//     fn parse_bool(&mut self) -> Result<Node, ParserError> {}
//     fn parse_null(&mut self) -> Result<Node, ParserError> {}

//     fn step(&mut self) {
//         self.index += 1
//     }
//     fn peek(&self) -> Option<&Token> {
//         self.tokens.get(self.index)
//     }
// }
