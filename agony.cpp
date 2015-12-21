#include "agony.hpp"
#include <fstream>
using namespace std;
Agony::Agony() : csize(10), cursor_x(0),
                 cursor_y(0), current_z(0),
                 m_y_sym_on(false), m_x_sym_on(false),
                 isDesignating(false), current_activity(0), mouse_is_over(false),isCircle(false) {
  m_vertz.setPrimitiveType(sf::Quads);
  fontthing.loadFromFile("LinLibertine_DRah.ttf");
  zz.setString("");
  zz.setFont(fontthing);
  zz.setColor(sf::Color(255, 200, 0));
  std::string a = "Designating:";
  a += (designations[current_activity] == '\0' ? 'x' : designations[current_activity]);
  zz.setString(a);
}
Agony::~Agony() {}
void Agony::draw(sf::RenderTarget &target, sf::RenderStates states) const {

  states.transform *= getTransform();
  states.texture = NULL;
  target.draw(m_vertz, states);
  //draw the cursor
  sf::RectangleShape r(sf::Vector2f(csize, csize));
  r.setOrigin(this->getOrigin());
  r.setFillColor(sf::Color(0, 0, 0, 0));
  r.setOutlineColor(sf::Color(255, 255, 0, 255));
  r.setOutlineThickness(1.0);
  r.move(csize * cursor_x, csize * cursor_y);
  //this is the x axis symmetry thing
  sf::RectangleShape xSym(sf::Vector2f(2, target.getSize().y));
  xSym.setOrigin(getOrigin());
  
  xSym.move(csize * xsym +csize/2 , 0);
  xSym.setFillColor(sf::Color(0, 255, 0, 255));
  //this is the y axis symmetry thing
  sf::RectangleShape ySym(sf::Vector2f(target.getSize().x, 2));
  ySym.setOrigin(getOrigin());
  ySym.move(0, ysym * csize+csize/2);
  ySym.setFillColor(sf::Color(255, 0, 0, 255));
  if (isDesignating) {
    sf::CircleShape start_desig(csize / 2);
    start_desig.setFillColor(sf::Color(255, 0, 255));
    start_desig.setOrigin(getOrigin());
    start_desig.move(m_start.x() * csize, m_start.y() * csize);
    target.draw(start_desig);
  }
  if (mouse_is_over) {
    sf::CircleShape q(csize / 2);
    q.setOrigin(getOrigin());
    q.move((int)mouse_place.x * csize, mouse_place.y * csize);
    q.setFillColor(sf::Color(0, 0, 0, 0));
    q.setOutlineColor(sf::Color(255, 255, 255, 255));
    q.setOutlineThickness(1);
    target.draw(q);
  }
  if (m_y_sym_on)
    target.draw(ySym);
  if (m_x_sym_on)
    target.draw(xSym);
  target.draw(r);
  zz.setOrigin(target.getSize().x / 2, target.getSize().y / 2);
  target.draw(zz);
}
void Agony::update() {
  std::vector<Eigen::Vector3d> things;
  std::vector<Eigen::Vector3d> torem;
  for (auto i : allowed) {
    if (std::round(i.first.z()) == current_z) {
      if (i.second != '\0') {
        things.push_back(i.first);
      } else {
        torem.push_back(i.first);
      }
    }
  }
  for (auto i : torem)
    allowed.erase(i);
  m_vertz.resize(things.size() * 4);

  m_vertz.setPrimitiveType(sf::PrimitiveType::Quads);
  for (int i = 0; i < things.size(); i++) {
    sf::Vertex *current = &m_vertz[i * 4];
    auto g = things[i];

    current[0].position = sf::Vector2f(g[0] * csize, g[1] * csize);
    current[1].position = sf::Vector2f((g[0] + 1) * csize, g[1] * csize);
    current[2].position = sf::Vector2f((1 + g[0]) * csize, (1 + g[1]) * csize);
    current[3].position = sf::Vector2f(g[0] * csize, (1 + g[1]) * csize);
    sf::Color curcol;
    switch (allowed[things[i]]) {
    case 'd':
      curcol = sf::Color(255, 255, 255);
      break;
    case 'j':
      curcol = sf::Color(255, 255, 0);
      break;
    case 'i':
      curcol = sf::Color(0, 255, 0, 255);
      break;
    case 'u':
      curcol = sf::Color(255, 0, 0, 255);
      break;
    case '\0':
      curcol = sf::Color(0, 0, 0);
      break;
    }
    current[0].color = curcol;
    current[1].color = curcol;
    current[2].color = curcol;
    current[3].color = curcol;
  }
}
void Agony::long_desig() {
  if (!isDesignating) {
    m_start = Eigen::Vector3d(cursor_x, cursor_y, current_z);
  } else {
    m_end = Eigen::Vector3d(cursor_x, cursor_y, current_z);
    if (isCircle) {
      draw_circle();
      isCircle = false;

    } else {
      if (m_end[0] < m_start[0])
        std::swap(m_end[0], m_start[0]);
      if (m_end[1] < m_start[1])
        std::swap(m_end[1], m_start[1]);
      if (m_end[2] < m_start[2])
        std::swap(m_end[2], m_start[2]);
      //std::cout<<allowed.size()<<"/";
      for (int i = m_start[0]; i <= m_end[0]; i++)
        for (int j = m_start[1]; j <= m_end[1]; j++)
          for (int k = m_start[2]; k <= m_end[2]; k++) {
            designate(i, j, k);
          }
      update();
    }
  }
  update_text();
  isDesignating = !isDesignating;
}
void Agony::insert_x_symmetry(const Eigen::Vector3d &c) {
  if (m_x_sym_on) {
    Eigen::Vector3d f;
    int diff = std::abs(c.x() - (xsym));
    if (c.x() <= xsym) {
      //std::cout << "low" << std::endl;
      f = Eigen::Vector3d((xsym)+diff, c.y(), c.z());
    } else {
      //std::cout << "high" << std::endl;
      f = Eigen::Vector3d((xsym)-diff, c.y(), c.z());
    }
    allowed[f] = designations[current_activity];
    insert_y_symmetry(f);
  }
  insert_y_symmetry(c);
}
void Agony::insert_y_symmetry(const Eigen::Vector3d &c) {

  if (m_y_sym_on) {
    Eigen::Vector3d f;
    int diff = std::abs(c.y() - (ysym));
    if (c.y() <= ysym) {
      f = Eigen::Vector3d(c.x(), (ysym)+diff, c.z());
    } else {
      f = Eigen::Vector3d(c.x(), (ysym)-diff, c.z());
    }
    allowed[f] = designations[current_activity];
  }
}
void Agony::designate(int x, int y, int z) {
  //std::cout<<x<<","<<y<<","<<z<<std::endl;
  Eigen::Vector3d c(x, y, z);
  allowed[c] = designations[current_activity];
  insert_x_symmetry(c);
}
void Agony::designate() {
  //std::cout << cursor_x << "," << cursor_y << std::endl;
  designate(cursor_x, cursor_y, current_z);
  update();
}
void Agony::mouse_over(const sf::Vector2f &e) {
  mouse_is_over = true;
  mouse_place.x = (int)std::round(e.x);
  mouse_place.y = (int)std::round(e.y);
}
void Agony::long_desig(const sf::Vector2f &e) {
  cursor_x = (int)std::round(e.x);
  cursor_y = (int)std::round(e.y);
  long_desig();
}
void Agony::increase_activity() {
  current_activity++;
  current_activity %= 5;
  update_text();
}
void Agony::decrease_activity() {
  if (current_activity == 0)
    current_activity = 4;
  else
    current_activity--;
  current_activity %= 5;
  update_text();
}
void Agony::update_text(){
  std::string a = "Designating";
  if(isCircle){
    a+="(Circle)";
  }
  a+=":";
  a += designations[current_activity] == '\0' ? 'x' : designations[current_activity];
  zz.setString(a);
}
void Agony::add_x_symmetry_at_cursor() {
  if (xsym != cursor_x || !m_x_sym_on) {
    m_xsym = make_Reflection(cursor_x, 1, 1);
    xsym = cursor_x;
    m_x_sym_on = true;
  } else {
    m_x_sym_on = false;
  }
}
void Agony::add_y_symmetry_at_cursor() {
  if (ysym != cursor_y || !m_y_sym_on) {
    ysym = cursor_y;
    m_y_sym_on = true;
  } else {
    m_y_sym_on = false;
  }
}
void Agony::set_thing(int x, int y, int z) {
  allowed.insert(std::pair<Eigen::Vector3d, int>(Eigen::Vector3d(x, y, z), designations[current_activity]));
  update();
}
void Agony::erase_position() {
  Eigen::Vector3d e(cursor_x, cursor_y, current_z);
  allowed.erase(e);
}
std::tuple<int, int, int, int, int, int> Agony::getBoundaries() const {
  int minx = 200000, miny = 200000, minz = 200000;
  int maxx = -200000, maxy = -200000, maxz = -200000;
  for (auto i : allowed) {
    auto z = i.first;
    if (z[0] < minx)
      minx = z[0];
    if (z[1] < miny)
      miny = z[1];
    if (z[2] < minz)
      minz = z[2];
    if (z[0] > maxx)
      maxx = z[0];
    if (z[1] > maxy)
      maxy = z[1];
    if (z[2] > maxz)
      maxz = z[2];
  }
  return std::make_tuple(minx, miny, minz, maxx, maxy, maxz);
}
void Agony::write_file_output(const std::string &output_name) const {
  auto boundaries = getBoundaries();
  ofstream out(output_name);
  out << "#dig" << endl;
  for (int z = std::get<5>(boundaries); z >= std::get<2>(boundaries); z--) {
    for (int y = std::get<1>(boundaries); y <= std::get<4>(boundaries); y++) {
      for (int x = std::get<0>(boundaries); x <= std::get<3>(boundaries); x++) {
        Eigen::Vector3d h = Eigen::Vector3d(x, y, z);
        if (allowed.find(h) != allowed.end()) {
          out << allowed.find(h)->second;
        } else {
          out << " ";
        }
        out << ",";
      }
      out << "#" << endl;
    }
    if (z > std::get<2>(boundaries) + 1)
      out << "#>" << endl;
  }
  out.close();
}
void Agony::set_circle() {
  this->isCircle = true;
  update_text();
}
float distance(const Eigen::Vector3d &a, const Eigen::Vector3d &b) {
  auto c = Eigen::Vector3d(a.x()-b.x(),a.y()-b.y(),a.z()-b.z());
  return std::sqrt(c.x() * c.x() + c.y() * c.y() + c.z() * c.z());
}
void Agony::draw_circle() {
  float xx=(m_start[0]-m_end[0]),
       yy=(m_start[1]-m_end[1]);
       
  int r=sqrt(xx*xx+yy*yy);
  std::cout<<r<<endl;
  float hy=m_start[1];
  float hx=m_start[0];
  std::cout<<"h="<<hx<<",k="<<hy<<endl;
  for (int x = hx-r-1; x <= hx+r+1; x++) {
    for (int y = hy-r-1; y <= hy+r+1; y++) {
      for (int z = m_start[2]; z <= m_end[2]; z++) {
        cout<<x<<","<<y<<endl;
        float v = ((x - hx) * (x - hx)) / (r * r) +
                  ((y - hy) * (y - hy)) / (r * r);
        if (v <= 1) {
          designate(x, y, z);
        }
      }
    }
  }
  update();
}
void Agony::move_over(int x,int y){
  auto g=getOrigin();
  g.x/=csize;
  g.y/=csize;
  setOrigin(g.x*csize+x*csize, g.y*csize+y*csize);
}