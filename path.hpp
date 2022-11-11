#ifndef PATH_H
#define PATH_H

#pragma once

#include "file_include.hpp"

class path_t {
public:
    using value_type = int;
    using edge_t = std::pair<value_type, value_type>;
    using way_t = std::vector<edge_t>;
    using distance_t = size_t;
    using matrix_t = std::vector<std::vector<value_type>>;
protected:
    static constexpr value_type inf = std::numeric_limits<value_type>::max();
    
    way_t way;
    distance_t distance;
public:
    path_t(size_t __reserv = 10) noexcept : way(), distance(0) {
        way.reserve(__reserv);
    }

    path_t(path_t && rhs) noexcept : way(std::move(rhs.way)), distance(rhs.distance) {    
        rhs.distance = 0;
    }

    path_t(const path_t & rhs) : way(rhs.way), distance(rhs.distance) {}

    void add_edge(const matrix_t & mx_dist, const edge_t& edge) {
        way.push_back(edge);
        distance += static_cast<distance_t>(mx_dist[edge.first][edge.second]);
    }   

    distance_t get_cost() const noexcept {
        return distance;
    } 

    const way_t & get_way() const noexcept {
        return way;
    }

    path_t& operator=(const path_t &rhs) {
        way = rhs.way;
        distance = rhs.distance;
        return *this;
    }

    path_t &operator=(path_t && rhs) noexcept {
        way = std::move(rhs.way);
        distance = rhs.distance;
        rhs.distance = 0;
        return *this;
    }

    ~path_t() = default;
};


#endif
