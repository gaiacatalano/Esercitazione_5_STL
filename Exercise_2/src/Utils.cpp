#include "Utils.hpp"
#include "PolygonalMesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

namespace PolygonalLibrary{

bool ImportMesh(const string& path, PolygonalMesh& mesh)
{

    if(!ImportCell0Ds(path + "/Cell0Ds.csv", mesh))
    {
        return false;
    }
    else
    {
        // check sul corretto salvataggio dei markers

        cout << "Cell0D marker: " << endl;
        for(auto it = mesh.Cell0DMarkers.begin(); it != mesh.Cell0DMarkers.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;
            cout << endl;
        }
    }

    if(!ImportCell1Ds(path + "/Cell1Ds.csv", mesh))
    {
        return false;
    }
    else
    {
        // check sul corretto salvataggio dei markers

        cout << "Cell1D marker: " << endl;
        for(auto it = mesh.Cell1DMarkers.begin(); it != mesh.Cell1DMarkers.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;
            cout << endl;
        }
    }

    if(!ImportCell2Ds(path + "/Cell2Ds.csv", mesh))
    {
        return false;
    }
    else
    {
        // check sul corretto salvataggio dei markers

        cout << "Cell2D marker: " << endl;
        for(auto it = mesh.Cell2DMarkers.begin(); it != mesh.Cell2DMarkers.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;
            cout << endl;
        }

        for(unsigned int c = 0; c < mesh.NumberCell2D; c++) //per ogni poligono prendo il c-esimo
        {
            vector<unsigned int> edges = mesh.Cell2DEdges[c]; //la componente c-esima del vettore Cell2DEdges contiene il vettore di lati del poligono c-esimo
            unsigned int ne = edges.size(); // ho tanti lati nel poligono quanto la dimensione del vettore che li contiene

            for(unsigned int e = 0; e < ne; e++)
            {
                // tra i lati, cerco il vertice corrispondente all'inizio e alla fine
                const unsigned int origin = mesh.Cell1DVertices[edges[e]][0]; //Cell1DVertices è un vettore di vettori di 2 interi (origin e end);
                const unsigned int end = mesh.Cell1DVertices[edges[e]][1];

                // controllo che i lati abbiamo lunghezza != 0

                if(origin == end)
                {
                    cerr << "Wrong mesh: there is an endge of lenght 0" << endl;
                    return 2;
                }

                // controllo che i vertici corrispondano agli estremi dei lati

                auto findOrigin = find(mesh.Cell2DVertices[c].begin(), mesh.Cell2DVertices[c].end(), origin);
                if(findOrigin == mesh.Cell2DVertices[c].end()) //se non la trovo ovvero arrivo alla fine della lista
                {
                    cerr << "Wrong mesh: vertex does not coorespond to the correct edge" << endl;
                    return 3;
                }

                auto findEnd = find(mesh.Cell2DVertices[c].begin(), mesh.Cell2DVertices[c].end(), end);
                if(findEnd == mesh.Cell2DVertices[c].end())
                {
                    cerr << "Wrong mesh: vertex does not coorespond to the correct edge" << endl;
                    return 4;
                }

                // controllo che ci siano almeno 3 lati

                if(mesh.Cell2DEdges[c].size()<3){
                    cerr << "Wrong mesh: there are less than 3 edges";
                    return 5;
                }

                // controllo che i lati siano tutti diversi

                int count = 0;
                for(unsigned int i = 0; i < mesh.Cell2DEdges[c].size(); i++){
                    if(mesh.Cell2DEdges[i] == mesh.Cell2DEdges[c])
                        count++;
                }
                if(count>1){
                    cerr << "Wrong mesh: the same edge has been added more than once";
                    return 6;
                }

                // controllo che l'area sia != 0

                vector<double> x = {};
                vector<double> y = {};

                for(unsigned int i = 0; i < mesh.Cell0DCoordinates.size(); i++)
                {
                    vector<Vector2d> v = mesh.Cell0DCoordinates;
                    x.push_back((v[i])[0]);
                    y.push_back((v[i])[1]);
                }

                double sum = 0;

                for(unsigned int i = 0; i < mesh.Cell0DCoordinates.size(); i++)
                {
                    if(i==mesh.Cell0DCoordinates.size()-1)
                    {
                        sum += abs((x[i]*y[0]) - (x[0]*y[i]));
                    }
                    else
                    {
                        sum += abs((x[i]*y[i+1]) - (x[i+1]*y[i]));
                    }
                }

                double area = 0.5*sum;
                if(area == 0)
                {
                    cerr << "Wrong mesh: the area of a polygon is null";
                    return 7;
                }

            }
        }
    }

    return true;
}

bool ImportCell0Ds(const string &filename, PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
    {
        return false;
    }

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }

    file.close();

    listLines.pop_front();

    mesh.NumberCell0D = listLines.size();

    if (mesh.NumberCell0D == 0)
    {
        cerr << "There is no cell 0D" << endl;
        return false;
    }

    mesh.Cell0DId.reserve(mesh.NumberCell0D);
    mesh.Cell0DCoordinates.reserve(mesh.NumberCell0D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2d coord;

        converter >>  id >> marker >> coord(0) >> coord(1);

        mesh.Cell0DId.push_back(id);
        mesh.Cell0DCoordinates.push_back(coord);

        auto ret = mesh.Cell0DMarkers.insert({marker, {id}});
        if(!ret.second)
            (ret.first)->second.push_back(id);

    }
    file.close();
    return true;
}

bool ImportCell1Ds(const string &filename, PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
    {
        return false;
    }

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        std::replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }

    file.close();

    listLines.pop_front();

    mesh.NumberCell1D = listLines.size();

    if (mesh.NumberCell1D == 0)
    {
        cerr << "There is no cell 1D" << endl;
        return false;
    }

    mesh.Cell1DId.reserve(mesh.NumberCell1D);
    mesh.Cell1DVertices.reserve(mesh.NumberCell1D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2i originend;

        converter >>  id >> marker >> originend(0) >> originend(1);

        mesh.Cell1DId.push_back(id);
        mesh.Cell1DVertices.push_back(originend);

        auto ret = mesh.Cell1DMarkers.insert({marker, {id}});
        if(!ret.second)
            (ret.first)->second.push_back(id);
    }


    file.close();
    return true;
}

bool ImportCell2Ds(const string &filename, PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        std::replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }

    listLines.pop_front();

    mesh.NumberCell2D = listLines.size();

    if (mesh.NumberCell2D == 0)
    {
        cerr << "There is no cell 2D" << endl;
        return false;
    }

    mesh.Cell2DId.reserve(mesh.NumberCell2D);
    mesh.Cell2DVertices.reserve(mesh.NumberCell2D);
    mesh.Cell2DEdges.reserve(mesh.NumberCell2D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        unsigned int numvertices;
        unsigned int numedges;

        converter >>  id  >> marker >> numvertices;
        vector<unsigned int> vertices(numvertices) ;

        for(unsigned int i = 0; i < numvertices; i++)
            converter >> vertices[i];

        converter >> numedges;
        vector<unsigned int> edges(numedges) ;

        for(unsigned int i = 0; i < numedges; i++)
            converter >> edges[i];

        mesh.Cell2DId.push_back(id);
        mesh.Cell2DVertices.push_back(vertices);
        mesh.Cell2DEdges.push_back(edges);

        if(marker != 0)
        {
        auto ret = mesh.Cell2DMarkers.insert({marker, {id}});
        if(!ret.second)
            (ret.first)->second.push_back(id);
        }

    }
    file.close();
    return true;
}
}
