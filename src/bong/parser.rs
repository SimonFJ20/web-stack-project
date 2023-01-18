use std::iter::Map;

use crate::bong::lexer::Lexer;

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

pub struct Parser<'a> {
    lexer: Lexer<'a>,
}

impl<'a> Parser<'a> {
    pub fn new(lexer: Lexer<'a>) -> Self {
        Self { lexer }
    }
}
