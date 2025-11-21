#include <iostream>
#include <string>
#include <vector>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using namespace std;

using json = nlohmann::json;


struct Character
{
  string name;
  string origin;
  string species;
  string status;

  vector<string> episodeURLs;
};

vector<Character> parse_characters(const string &body)
{
  vector<Character> characters;

  json j = json::parse(body);

  if (!j.contains("results") || !j["results"].is_array())
  {
    return characters;
  }

  for (const auto &item : j["results"])
  {
    Character c;

    c.name = item.value("name", "Desconocido");
    c.status = item.value("status", "Desconocido");
    c.species = item.value("species", "Desconocida");

    if (item.contains("origin") && item["origin"].is_object())
    {
      c.origin = item["origin"].value("name", "Desconocido");
    }
    else
    {
      c.origin = "Desconocido";
    }

    if (item.contains("episode") && item["episode"].is_array())
      c.episodeURLs = item["episode"].get<vector<string>>();

    characters.push_back(move(c));
  }

  return characters;
}

int main()
{
  cout << "Introduce el nombre (o parte del nombre) del personaje de Rick & Morty: ";
  string search;
  getline(cin, search);

  if (search.empty())
  {
    cerr << "Cadena de búsqueda vacía. Terminando.\n";
    return 1;
  }

  // Petición GET con cpr, usando parámetros (hace URL encoding por ti)
  auto response = cpr::Get(
      cpr::Url{"https://rickandmortyapi.com/api/character/"},
      cpr::Parameters{{"name", search}});

  if (response.error)
  {
    cerr << "Error en la petición HTTP: " << response.error.message << "\n";
    return 1;
  }

  if (response.status_code != 200)
  {
    cerr << "La API devolvió código HTTP "
              << response.status_code << "\nCuerpo:\n"
              << response.text << "\n";
    return 1;
  }

  auto characters = parse_characters(response.text);
  if (characters.empty())
  {
    cout << "No se encontraron resultados para: " << search << "\n";
    return 0;
  }

  // Mostrar listado de resultados
  cout << "\nResultados encontrados:\n";
  for (size_t i{0}; i < characters.size(); ++i)
  {
    cout << i << ") " << characters.at(i).name << '\n';
  }

  // Seleccionar uno
  cout << "\nSelecciona el índice del personaje que te interesa: ";
  size_t index = 0;
  if (!(cin >> index))
  {
    cerr << "Entrada no válida.\n";
    return 1;
  }

  if (index >= characters.size())
  {
    cerr << "Índice fuera de rango.\n";
    return 1;
  }

  const auto &c = characters.at(index);

  cout << "\n--- Detalles del personaje ---\n";
  cout << "Nombre : " << c.name << '\n';
  cout << "Planeta (origen): " << c.origin << '\n';
  cout << "Especie: " << c.species << '\n';
  cout << "Status : " << c.status << '\n';


  /////////////////////////////////////////////////////////////////////////////////////////////////////

  // Petición GET con cpr, usando parámetros (hace URL encoding por ti)

  cout << "Episodios: " << endl;

  for (auto episodeURL : c.episodeURLs) {

    auto response = cpr::Get(cpr::Url{episodeURL});

    if (response.error)
    {
      cerr << "Error en la petición HTTP: " << response.error.message << "\n";
      return 1;
    }

    if (response.status_code != 200)
    {
      cerr << "La API devolvió código HTTP "
                << response.status_code << "\nCuerpo:\n"
                << response.text << "\n";
      return 1;
    }

    json j = json::parse(response.text);

    cout << " " << j.value("name", "Desconocido") << endl;

  }

  return 0;
}
