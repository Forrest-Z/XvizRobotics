/**
 * @Author: Xia Yunkai
 * @Date:   2024-01-27 23:29:04
 * @Last Modified by:   Xia Yunkai
 * @Last Modified time: 2024-02-05 20:14:22
 */
#ifndef __PARKING_CASE_PARSER_H__
#define __PARKING_CASE_PARSER_H__
#include "common/data_types.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "common/calculations.h"
#include "map/grid_map.h"
#include "map/esdf_map.h"
#include "common/polygon_collide.h"
namespace parking_case_parser
{

  class ParkingCase
  {

  public:
    bool Parse(const std::string &path)
    {

      std::vector<float> mapData;
      if (!ReadCsv(path, mapData))
      {
        return false;
      }
      m_startPose[0] = mapData[0];
      m_startPose[1] = mapData[1];
      m_startPose[2] = mapData[2];
      m_tarPose[0] = mapData[3];
      m_tarPose[1] = mapData[4];
      m_tarPose[2] = mapData[5];
      m_maxX = common::Max(m_startPose[0], m_tarPose[0]);
      m_maxY = common::Max(m_startPose[1], m_tarPose[1]);
      m_minX = common::Min(m_startPose[0], m_tarPose[0]);
      m_minY = common::Min(m_startPose[1], m_tarPose[1]);

      int obs_num = static_cast<int>(mapData[6]);
      int offset = 6 + obs_num;
      m_obs.clear();
      for (int i = 1; i <= obs_num; i++)
      {
        common::Polygon2f ob;

        int point_num = mapData[6 + i];
        for (int j = 1; j <= point_num; ++j)
        {
          common::Vec2f point;

          point[0] = mapData[offset + 2 * j - 1];
          point[1] = mapData[offset + 2 * j];
          m_maxX = common::Max(m_maxX, point[0]);
          m_maxY = common::Max(m_maxY, point[1]);
          m_minX = common::Min(m_minX, point[0]);
          m_minY = common::Min(m_minY, point[1]);
          ob.emplace_back(point);
        }
        m_obs.emplace_back(ob);

        offset += 2 * point_num;
      }

      float map_resolution = 0.05f;
      int width = int((m_maxX - m_minX) / map_resolution);
      int height = int((m_maxY - m_minY) / map_resolution);
      int data_size = width * height;
      char *data = new char[data_size];
      memset(data, 0, data_size * sizeof(char));
      common::Rigid2f origin(m_minX, m_minY, 0.0f);
      common::Vec2i map_size(width, height);
      m_map = std::make_shared<map::GridMap>();
      m_map->Init(origin, map_size, data, map_resolution);
      std::cout << " m_map->Init" << std::endl;
      delete[] data;

      for (auto &ob : m_obs)
      {
        float ob_max_x{ob.front().x()}, ob_max_y{ob.front().y()}, ob_min_x{ob.front().x()}, ob_min_y{ob.front().y()};

        for (auto &point : ob)
        {
          ob_max_x = common::Max(ob_max_x, point.x());
          ob_max_y = common::Max(ob_max_y, point.y());
          ob_min_x = common::Min(ob_min_x, point.x());
          ob_min_y = common::Min(ob_min_y, point.y());
        }
        common::Vec2f ob_max_pt{ob_max_x, ob_max_y},
            ob_min_pt{ob_min_x, ob_min_y};
        auto ob_min_pn = m_map->FloatToInt(ob_min_pt);
        auto ob_max_pn = m_map->FloatToInt(ob_max_pt);

        for (int i = ob_min_pn[1]; i <= ob_max_pn[1]; i++)
        {
          for (int j = ob_min_pn[0]; j <= ob_max_pn[0]; ++j)
          {
            common::Vec2f point = m_map->IntToFloat(common::Vec2i(j, i));
            if (common::PtInPolygon(point, ob))
            {
              m_map->SetOccupied(common::Vec2i(j, i));
              // std::cout << "SetOccupied" << std::endl;
            }
          }
        }
      }

      return true;
    }

    bool ReadCsv(const std::string &file_name, std::vector<float> &data)
    {
      data.clear();
      std::ifstream in_file(file_name, std::ios::in);
      if (!in_file.is_open())
      {
        std::cout << "can not open fine" << std::endl;
        return false;
      }
      std::string line;
      std::string word;
      std::stringstream ss;
      getline(in_file, line);
      ss << line;
      while (getline(ss, word, ','))
      {
        double temp;
        data.push_back(stof(word));
      }
      in_file.close();
      return true;
    }

    const map::GridMap::Ptr GetMap() const { return m_map; }

  public:
    common::Vec3f m_startPose;
    common::Vec3f m_tarPose;
    std::vector<common::Polygon2f> m_obs;
    float m_maxX, m_maxY, m_minX, m_minY;
    map::GridMap::Ptr m_map;
  };

} // namespace parking_case_parser

#endif /* __PARKING_CASE_PARSER_H__ */
