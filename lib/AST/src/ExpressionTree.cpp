#include "ExpressionTree.h"
#include <iostream>
ExpressionTree::ExpressionTree(ElementMap& gameState, std::shared_ptr<PlayerMap> playerMap)
    : gameState(gameState), playerMap(playerMap) { }

std::shared_ptr<ASTNode> ExpressionTree::getRoot(){
    return root;
}

int ExpressionTree::getPrecedence(std::string key){
    return precedenceMap.at(key);
}

bool isNum(std::string value){
    return std::find_if(value.begin(), value.end(), 
        [](unsigned char c) { return !std::isdigit(c); }) == value.end();
}

ElementSptr inGameState(std::string token, ElementMap& gameState){
    auto gameList = gameState.find(token);
    if(gameList != gameState.end()){
        return gameList->second;
    }
    else{
        for(auto& [name, list] : gameState){
            ElementSptr element;
            //if we stored value of anything in setup, it cannot be changed by owner after creation
            if(name != "setup")
                element = list->getMapElement(token);
            if( element != nullptr)
                return element;
        }
    }
    return nullptr;
}

ExpressionTree::nodeType ExpressionTree::getType(std::string token){
    if(token == "(") 
        return OPEN_BRACE;
    else if(token == ")") 
        return CLOSE_BRACE;
    else if(token == "players") 
        return PLAYERS;
    else if(isNum(token)) 
        return NUMBER;
    else if(std::find(binaryOperators.begin(), binaryOperators.end(), token) != binaryOperators.end())
        return BINARY;
    else if(std::find(unaryOperators.begin(), unaryOperators.end(), token) != unaryOperators.end())
        return UNARY;
    else if(std::find(ternaryOperators.begin(), ternaryOperators.end(), token) != ternaryOperators.end())
        return TERNARY;
    else if(inGameState(token, gameState) != nullptr)
        return LIST;
    else 
        return NAME;
}

void ExpressionTree::build(std::string expression){
    operatorStack.clear();
    nodeStack.clear();

    std::deque<std::string> tokens = splitString(expression);
    if(tokens.size() == 0)
        return;
    tokens.emplace_front("(");
    tokens.emplace_back(")");

    std::string prevToken;

    for(auto token : tokens){

        switch(getType(token)){

            case OPEN_BRACE: {
                operatorStack.emplace_back(token);
                break;
            }

            case CLOSE_BRACE: {
                while( !operatorStack.empty() && getType(operatorStack.back()) != OPEN_BRACE) {
                    buildOperatorNode(getType(operatorStack.back()));
                }
                operatorStack.pop_back();
                break;
            }

            case LIST: {
                auto element = inGameState(token, gameState);
                root = std::make_shared<ListNode>(token, element);
                nodeStack.emplace_back(std::move(root));
                break;
            }

            case PLAYERS: {
                root = std::make_shared<PlayersNode>(playerMap);
                nodeStack.emplace_back(std::move(root));
                break;
            }
            
            case NUMBER: {
                int num = stoi(token);
                root = std::make_shared<NumberNode>(num);
                nodeStack.emplace_back(std::move(root));
                break;
            }
            //else it is just a string
            case NAME: {
                root = std::make_shared<NameNode>(token);
                nodeStack.emplace_back(std::move(root));
                break;
            }
            //if token is operator
            default: {
                //ignore "." if it directly precedes operator
                if(prevToken == ".")
                    operatorStack.pop_back();
                    
                while (!operatorStack.empty() && getType(operatorStack.back()) != OPEN_BRACE && getPrecedence(operatorStack.back()) >= getPrecedence(token) ){
                    buildOperatorNode(getType(operatorStack.back()));
                }
                operatorStack.emplace_back(token);
                break;
            }
        }

        prevToken = token;
    }
   
    root = std::move(nodeStack.back());
}

void ExpressionTree::buildOperatorNode(nodeType type) {
    switch(type){
        case UNARY: {
            auto operand = nodeStack.back();
            nodeStack.pop_back();
            root = std::make_shared<UnaryOperator>(operatorStack.back(), operand);
            
            operatorStack.pop_back();
            nodeStack.emplace_back(std::move(root));
            break;
        }
        case TERNARY: {
            auto right = nodeStack.back();
            nodeStack.pop_back();
            auto middle = nodeStack.back();
            nodeStack.pop_back();
            auto left= nodeStack.back();
            nodeStack.pop_back();

            root = std::make_shared<TernaryOperator>(operatorStack.back(), left, middle, right);
            operatorStack.pop_back();
            nodeStack.emplace_back(std::move(root));
            break;
        }
        case BINARY: {
            auto right = nodeStack.back();
            nodeStack.pop_back();
            auto left= nodeStack.back();
            nodeStack.pop_back();

            root = std::make_shared<BinaryOperator>(operatorStack.back(), left, right);
            operatorStack.pop_back();
            nodeStack.emplace_back(std::move(root));
            break;
        }
        default:
            break;
    }
}


std::deque<std::string> ExpressionTree::splitString(std::string expression) {
    int pos = 0;
    bool alnum = false;
    bool punct = false;
    std::deque<std::string> splits;

    for (unsigned i = 0; i < expression.length(); i++) {
        if (isalnum(expression[i])) {
            if (punct) {
                splits.emplace_back(expression.substr(pos, i-pos));
                punct = false;
            }
            if (!alnum) {
                pos = i;
                alnum = true;
            }
        } else if (ispunct(expression[i]) && !(expression[i] == ',')) {
            if (alnum) {
                splits.emplace_back(expression.substr(pos, i-pos));
                alnum = false;
            }
            if (!punct) {
                pos = i;
                punct = true;
            }
        } else {
            if (alnum || punct) {
                splits.emplace_back(expression.substr(pos, i-pos));
                alnum = punct = false;
            }
        }
    }
    if (alnum || punct) {
        splits.emplace_back(expression.substr(pos, expression.length()-pos));
    }

    return splits;
}
