#pragma once 
#ifndef ADJACENCY_LIST_H
#define ADJACENCY_LIST_H
#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <unordered_set>
#include "linear_equation.h"


class adjacency_list
{
public:
    using value_type = size_t;
    using adj_list_t = std::list<size_t>;
    using container = std::vector<adj_list_t>;
    using edges_t = std::vector<std::pair<size_t, size_t>>;
private:
    container vec_adj; 

    container create_vec_adj(const edges_t& edges, size_t sz) {
        container vec_res(sz);
        for(const auto& [from, to] : edges) {
            vec_res[from].push_back(to);
            vec_res[to].push_back(from);
        }
        return vec_res;
    }

public:
    adjacency_list(const edges_t& edges, size_t __count_vertex) : vec_adj(create_vec_adj(edges, __count_vertex)) {}
    
    adjacency_list(size_t sz) : vec_adj(sz) {}

    adjacency_list(const adjacency_list& lst) : vec_adj(lst.vec_adj) {}
    
    ~adjacency_list() {}

    adjacency_list&operator=(const adjacency_list& lst) {
        if (this == &lst) {
            return *this;
        }
        vec_adj = lst.vec_adj;
        return *this;
    }

    adj_list_t& operator[](size_t i) {
        return vec_adj[i];  
    }

    const adj_list_t& operator[](size_t i) const {
        return vec_adj[i];
    }

    size_t size() const noexcept {
        return vec_adj.size();
    }

    void add(size_t from, size_t to) {
        vec_adj[from].push_back(to);
        vec_adj[to].push_back(from);
    }
    
    void erase(size_t from, size_t to) {
        vec_adj[from].erase(std::find(std::begin(vec_adj[from]), std::end(vec_adj[from]), to));        
    } 
    
    void print() const {
        size_t i = 0;

        for(const auto& li : vec_adj) {
            std::cout <<  i << " -> [ ";
            for(const auto& el : li) {
                std::cout << el << " ";
            }
            ++i;
            std::cout << "]" << std::endl;
        }
    }


};

decltype(auto) get_add_restrictions(const adjacency_list& adj_li) {
    std::vector<linear_equation<EQUAL>> vec_res;
    for(size_t i = 0; i < adj_li.size(); ++i) {
        const auto& li = adj_li[i];
        if (li.size() > 2) {
            std::vector<std::pair<size_t, size_t>> tmp;
            for(const auto & vertex : li) {
                tmp.emplace_back(i, vertex);
            }

            vec_res.push_back(linear_equation<EQUAL>(std::move(tmp), 2));
        }
    }
    return vec_res;
}



/// @brief функция, основанная на обходе в ширину, ищет компоненты графа 
/// @param adj_li список смежности графа 
/// @return список множеств, на которые распадается граф 
std::vector<std::list<size_t>> graph_component(const adjacency_list& adj_li) {
    using vertex_t = size_t;    
    enum color {WHITE, GRAY, BLACK};
    
    std::unordered_set<vertex_t> set_visist;
    for(size_t i = 0; i < adj_li.size(); ++i) {
        set_visist.insert(i);
    }

    std::vector<color> vec_color(adj_li.size(), WHITE);

    std::vector<std::list<size_t>> set_vertex;    
    
    for(size_t i = 0; !set_visist.empty(); ++i) {
        set_vertex.push_back(std::list<size_t>{});

        vec_color[*(set_visist.begin())] = GRAY;
        std::queue<vertex_t> q;
        q.push(*(set_visist.begin()));

        while (!q.empty()) {
            vertex_t u = q.front();
            const auto& ref_adj = adj_li[u];
            for(const auto& v : ref_adj) {
                if (vec_color[v] == WHITE) {
                    vec_color[v] = GRAY;
                    q.push(v);
                }
            }
            q.pop();
            vec_color[u] = BLACK;
            set_visist.erase(u);
            set_vertex[i].push_back(u);
        }
    }
    return set_vertex;
}


#endif