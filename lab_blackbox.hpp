#pragma once

//-------------------------------------------------------------------------------------------------
// Descriere:	nu va intereseaza momentan, va fi explicat in laboratoare viitoare
//				este folosit ca sa simplificam procesul de predare de tehnici de grafica, altfel ati fi
//				nevoiti sa invatati totul deodata. Asa ne concentram intai pe concepte cheie si apoi
//				o sa ajungem sa nu fim nevoiti sa folosim acest fisier.
//
// Nota:
//		desigur, puteti sa observati
//
// Autor: Lucian Petrescu
// Data: 28 Sep 2015
//-------------------------------------------------------------------------------------------------

//incarcator de shadere
#include "lab_shader_loader.hpp"
#include "lab_glut.hpp"

#pragma once
#include "dependente\glew\glew.h"

// GLM
#include "dependente\glm\glm.hpp"
#include "dependente\glm\gtc\type_ptr.hpp"
#include "dependente\glm\gtc\matrix_transform.hpp"


#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace lab
{
	struct Mesh
	{
		public:
			Mesh(unsigned int vbo, unsigned ibo, unsigned vao, unsigned count)
			{
				this->vbo = vbo; this->ibo = ibo; this->vao = vao; this->count = count;
				r = 1.0f;  g = b = 0.0f;//red
			}

			~Mesh()
			{
				//distruge mesh incarcat
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(1, &vbo);
				glDeleteBuffers(1, &ibo);
				//nu distrug program pt ca nu am ownership.. state machine design
			}

			//seteaza culoarea obiectului
			void setColor(float red, float green, float blue)
			{
				r = red; g = green; b = blue;
			}

			unsigned int vbo, ibo, vao, count;
			float r, g, b;
	};

	class BlackBox
	{
	public:
		BlackBox()
		{
			glEnable(GL_DEPTH_TEST);
			
			LoadShader();

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	//forced wire, no illumination -> no surface shading

			matrix_model = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
			matrix_view = glm::lookAt(glm::vec3(0, 0, 50), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

			setModelMatrix(matrix_model);
			setViewMatrix(matrix_view);
		}

		~BlackBox()
		{
			glDeleteProgram(gl_program_shader);
		}

		void LoadShader() 
		{
			// incarca un shader din fisiere si gaseste locatiile matricilor relativ la programul creat
			gl_program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");

			loc_model_matrix	= glGetUniformLocation(gl_program_shader, "Model");
			loc_view_matrix		= glGetUniformLocation(gl_program_shader, "View");
			loc_projection_matrix = glGetUniformLocation(gl_program_shader, "Projection");
			loc_resolution		= glGetUniformLocation(gl_program_shader, "resolution");

			glUseProgram(gl_program_shader);
		}
		
		// Load Mesh
		Mesh* loadMesh(const std::string& filename)
		{
			unsigned int vbo, ibo, vao, count;
			loadObj(filename, vao, vbo, ibo, count);
			Mesh *m = new Mesh(vbo, ibo, vao, count);
			return m;
		}
		
		void drawMesh(Mesh *mesh)
		{
			if (mesh)
			{
				glUniform3f(glGetUniformLocation(gl_program_shader, "color"), mesh->r, mesh->g, mesh->b);
				glBindVertexArray(mesh->vao);
				glDrawElements(GL_TRIANGLES, mesh->count, GL_UNSIGNED_INT, 0);
			}
			else std::cout << "!!!!!!!!!!! ERROR : null mesh" << std::endl;
		}

		void setModelMatrix(const glm::mat4 &model_matrix)
		{
			glUniformMatrix4fv(loc_model_matrix, 1, false, glm::value_ptr(model_matrix));
		}

		void setViewMatrix(const glm::mat4 &camera_matrix)
		{
			matrix_view = camera_matrix;
			glUniformMatrix4fv(loc_view_matrix, 1, false, glm::value_ptr(matrix_view));
		}

		void setProjectionMatrix(const glm::mat4 &projection_matrix)
		{
			matrix_projection = projection_matrix;
			glUniformMatrix4fv(loc_projection_matrix, 1, false, glm::value_ptr(matrix_projection));
		}

		//horribly inefficient..
		void drawAxisSystem(const glm::vec3& pos, const glm::vec3& right, const glm::vec3& up, const glm::vec3& fwd, float axis_length, float axis_width)
		{
			struct AxisVertexFormat
			{
				glm::vec3 position, color;
				AxisVertexFormat(const glm::vec3 &p, const glm::vec3 &c)
				{
					position = p; color = c;
				}
			};

			unsigned int vao, vbo, ibo;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glGenBuffers(1, &ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			std::vector<unsigned int> indices; for (unsigned int i = 0; i<6; i++) indices.push_back(i);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AxisVertexFormat), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AxisVertexFormat), (void*)(sizeof(float)* 3));
			
			glBindVertexArray(vao);
			std::vector<AxisVertexFormat> vertices;
			vertices.push_back(AxisVertexFormat(pos, glm::vec3(0, 0, 1)));
			vertices.push_back(AxisVertexFormat(pos + glm::normalize(fwd)*axis_length, glm::vec3(0, 0, 1)));
			vertices.push_back(AxisVertexFormat(pos, glm::vec3(1, 0, 0)));
			vertices.push_back(AxisVertexFormat(pos + glm::normalize(right)*axis_length, glm::vec3(1, 0, 0)));
			vertices.push_back(AxisVertexFormat(pos, glm::vec3(0, 1, 0)));
			vertices.push_back(AxisVertexFormat(pos + glm::normalize(up)*axis_length, glm::vec3(0, 1, 0)));
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(AxisVertexFormat), &vertices[0], GL_STATIC_DRAW);

			glLineWidth(axis_width);
			glDisable(GL_DEPTH_TEST);
			glBindVertexArray(vao);
			glUniform3f(glGetUniformLocation(gl_program_shader, "color"), 1,0,0);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)0);
			glUniform3f(glGetUniformLocation(gl_program_shader, "color"), 0, 1, 0);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int)* 2));
			glUniform3f(glGetUniformLocation(gl_program_shader, "color"), 0, 0, 1);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int)* 4));
			glEnable(GL_DEPTH_TEST);
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ibo);
		}

		//draw pt blackbox
		void notifyDisplay()
		{
			glClearDepth(1);
			glClear(GL_DEPTH_BUFFER_BIT);

			unsigned int time = glutGet(GLUT_ELAPSED_TIME);
			frameTime = time - previousTime;
			previousTime = time;
		}

		//reshape pt blackbox
		void notifyReshape(unsigned int width, unsigned int height)
		{
			//reshape
			if (height == 0)
				height = 1;

			window_width = width;
			window_height = height;

			matrix_projection = glm::perspective(90.0f, (float)width / height, 0.1f, 500.f);
			setProjectionMatrix(matrix_projection);
		}

		float getFrameTimeSeconds()
		{
			return frameTime / 1000.0f;
		}

	private:
		
		unsigned int frameTime;
		unsigned int previousTime;
		unsigned int gl_program_shader;
		unsigned int window_width, window_height;
		glm::mat4 matrix_model, matrix_view, matrix_projection;

		unsigned int loc_model_matrix;
		unsigned int loc_view_matrix;
		unsigned int loc_projection_matrix;
		unsigned int loc_resolution;


		unsigned int loadShader(const std::string &vertex_shader_string, const std::string &fragment_shader_string)
		{
			std::vector<unsigned int> shaders;
			shaders.push_back(_createShader(vertex_shader_string, GL_VERTEX_SHADER));
			shaders.push_back(_createShader(fragment_shader_string, GL_FRAGMENT_SHADER));
			return _createProgram(shaders);
		}
		
		unsigned int _createShader(const std::string &shader_code, GLenum shader_type)
		{
			int info_log_length = 0, compile_result = 0;
			unsigned int gl_shader_object;

			//construieste un obiect de tip shader din codul incarcat
			gl_shader_object = glCreateShader(shader_type);
			const char *shader_code_ptr = shader_code.c_str();
			const int shader_code_size = shader_code.size();
			glShaderSource(gl_shader_object, 1, &shader_code_ptr, &shader_code_size);
			glCompileShader(gl_shader_object);
			glGetShaderiv(gl_shader_object, GL_COMPILE_STATUS, &compile_result);

			//daca exista erori output la consola
			if (compile_result == GL_FALSE){
				std::string str_shader_type = "";
				if (shader_type == GL_VERTEX_SHADER) str_shader_type = "vertex shader";
				if (shader_type == GL_TESS_CONTROL_SHADER) str_shader_type = "tess control shader";
				if (shader_type == GL_TESS_EVALUATION_SHADER) str_shader_type = "tess evaluation shader";
				if (shader_type == GL_GEOMETRY_SHADER) str_shader_type = "geometry shader";
				if (shader_type == GL_FRAGMENT_SHADER) str_shader_type = "fragment shader";
				if (shader_type == GL_COMPUTE_SHADER) str_shader_type = "compute shader";

				glGetShaderiv(gl_shader_object, GL_INFO_LOG_LENGTH, &info_log_length);
				std::vector<char> shader_log(info_log_length);
				glGetShaderInfoLog(gl_shader_object, info_log_length, NULL, &shader_log[0]);
				std::cout << "Shader Loader: EROARE DE COMPILARE pentru " << str_shader_type << std::endl << &shader_log[0] << std::endl;
				return 0;
			}

			return gl_shader_object;
		}

		// creaza un program de tip shader
		unsigned int _createProgram(const std::vector<unsigned int> &shader_objects)
		{
			int info_log_length = 0, link_result = 0;

			//build OpenGL program object and link all the OpenGL shader objects
			unsigned int gl_program_object = glCreateProgram();
			for (std::vector<unsigned int>::const_iterator it = shader_objects.begin(); it != shader_objects.end(); it++) glAttachShader(gl_program_object, (*it));
			glLinkProgram(gl_program_object);
			glGetProgramiv(gl_program_object, GL_LINK_STATUS, &link_result);

			//if we get link errors log them
			if (link_result == GL_FALSE)
			{
				glGetProgramiv(gl_program_object, GL_INFO_LOG_LENGTH, &info_log_length);
				std::vector<char> program_log(info_log_length);
				glGetProgramInfoLog(gl_program_object, info_log_length, NULL, &program_log[0]);
				std::cout << "Shader Loader : EROARE DE LINKARE" << std::endl << &program_log[0] << std::endl;
				return 0;
			}

			//delete the shader objects because we do not need them any more (this should be re-written if usage of separate shader objects is intended!)
			for (std::vector<unsigned int>::const_iterator it = shader_objects.begin(); it != shader_objects.end(); it++) glDeleteShader((*it));

			return gl_program_object;
		}

		struct VertexFormat
		{
			float position_x, position_y, position_z;				//pozitia unui vertex (x,y,z)	
			float normal_x, normal_y, normal_z;						//vom invata ulterior, nu este folosit in acest lab
			float texcoord_x, texcoord_y;							//vom invata ulterior, nu este folosit in acest lab

			VertexFormat()
			{
				position_x = position_y = position_z = 0;
				normal_x = normal_y = normal_z = 0;
				texcoord_x = texcoord_y = 0;
			}

			VertexFormat(float px, float py, float pz)
			{
				position_x = px;
				position_y = py;
				position_z = pz;

				normal_x = normal_y = normal_z = 0;

				texcoord_x = texcoord_y = 0;
			}

			VertexFormat(float px, float py, float pz, float nx, float ny, float nz)
			{
				position_x = px;
				position_y = py;
				position_z = pz;

				normal_x = nx;
				normal_y = ny;
				normal_z = nz;

				texcoord_x = texcoord_y = 0;
			}

			VertexFormat(float px, float py, float pz, float tx, float ty)
			{
				position_x = px;		position_y = py;		position_z = pz;
				texcoord_x = tx;		texcoord_y = ty;
				normal_x = normal_y = normal_z = 0;
			}

			VertexFormat(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty)
			{
				position_x = px;
				position_y = py;
				position_z = pz;

				normal_x = nx;
				normal_y = ny;
				normal_z = nz;

				texcoord_x = tx;
				texcoord_y = ty;
			}

			VertexFormat operator=(const VertexFormat &rhs)
			{
				position_x = rhs.position_x;	
				position_y = rhs.position_y;	
				position_z = rhs.position_z;
				
				normal_x = rhs.normal_x;
				normal_y = rhs.normal_y;
				normal_z = rhs.normal_z;
				
				texcoord_x = rhs.texcoord_x;
				texcoord_y = rhs.texcoord_y;
				return (*this);
			}
		};

		//incarca un fisier de tip Obj (fara NURBS, fara materiale)
		//returneaza in argumentele trimise prin referinta id-ul OpenGL pentru vao(Vertex Array Object), pentru vbo(Vertex Buffer Object) si pentru ibo(Index Buffer Object)
		void loadObj(const std::string &filename, unsigned int &vao, unsigned int& vbo, unsigned int &ibo, unsigned int &num_indices)
		{
			//incarca vertecsii si indecsii din fisier
			std::vector<VertexFormat> vertices;
			std::vector<unsigned int> indices;
			_loadObjFile(filename, vertices, indices);

			std::cout << "Mesh Loader : am incarcat fisierul " << filename << std::endl;

			//creeaza obiectele OpenGL necesare desenarii
			unsigned int gl_vertex_array_object, gl_vertex_buffer_object, gl_index_buffer_object;

			//vertex array object -> un obiect ce reprezinta un container pentru starea de desenare
			glGenVertexArrays(1, &gl_vertex_array_object);
			glBindVertexArray(gl_vertex_array_object);

			//vertex buffer object -> un obiect in care tinem vertecsii
			glGenBuffers(1, &gl_vertex_buffer_object);
			glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer_object);
			glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(VertexFormat), &vertices[0], GL_STATIC_DRAW);

			//index buffer object -> un obiect in care tinem indecsii
			glGenBuffers(1, &gl_index_buffer_object);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_object);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			//legatura intre atributele vertecsilor si pipeline, datele noastre sunt INTERLEAVED.
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);						//trimite pozitii pe pipe 0
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(float)* 3));		//trimite normale pe pipe 1
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(float)* 3));	//trimite texcoords pe pipe 2

			vao = gl_vertex_array_object;
			vbo = gl_vertex_buffer_object;
			ibo = gl_index_buffer_object;
			num_indices = indices.size();
		}

		//helper funcs
		float _stringToFloat(const std::string &source){
			std::stringstream ss(source.c_str());
			float result;
			ss >> result;
			return result;
		}
		//transforms a string to an int
		unsigned int _stringToUint(const std::string &source)
		{
			std::stringstream ss(source.c_str());
			unsigned int result;
			ss >> result;
			return result;
		}
		
		//transforms a string to an int
		int _stringToInt(const std::string &source)
		{
			std::stringstream ss(source.c_str());
			int result;
			ss >> result;
			return result;
		}

		//writes the tokens of the source string into tokens
		void _stringTokenize(const std::string &source, std::vector<std::string> &tokens)
		{
			tokens.clear();
			std::string aux = source;
			for (unsigned int i = 0; i<aux.size(); i++) if (aux[i] == '\t' || aux[i] == '\n') aux[i] = ' ';
			std::stringstream ss(aux, std::ios::in);
			while (ss.good()){
				std::string s;
				ss >> s;
				if (s.size()>0) tokens.push_back(s);
			}
		}

		//variant for faces
		void _faceTokenize(const std::string &source, std::vector<std::string> &tokens)
		{
			std::string aux = source;
			for (unsigned int i = 0; i<aux.size(); i++) if (aux[i] == '\\' || aux[i] == '/') aux[i] = ' ';
			_stringTokenize(aux, tokens);
		}

		//incarca doar geometrie dintr-un fisier obj (nu incarca high order surfaces, materiale, coordonate extra, linii)
		// Format: http://paulbourke.net/dataformats/obj/
		//nu calculeaza normale sau coordonate de textura sau tangente, performanta neoptimala dar usor de citit (relativ la alte parsere..)
		//considera geometria ca pe un singur obiect, deci nu tine cont de grupuri sau de smoothing
		//daca apar probleme la incarcare fisier(?) incarcati mesha originala in meshlab(free!), salvati si folositi varianta noua.
		void _loadObjFile(const std::string &filename, std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices){
			//citim din fisier
			std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
			if (!file.good()){
				std::cout << "Mesh Loader: Nu am gasit fisierul obj " << filename << " sau nu am drepturile sa il deschid!" << std::endl;
				std::terminate();
			}

			std::string line;
			std::vector<std::string> tokens, facetokens;
			std::vector<glm::vec3> positions;		positions.reserve(1000);
			std::vector<glm::vec3> normals;		normals.reserve(1000);
			std::vector<glm::vec2> texcoords;		texcoords.reserve(1000);
			while (std::getline(file, line)){
				//tokenizeaza linie citita
				_stringTokenize(line, tokens);

				//daca nu am nimic merg mai departe
				if (tokens.size() == 0) continue;

				//daca am un comentariu merg mai departe
				if (tokens.size()>0 && tokens[0].at(0) == '#') continue;

				//daca am un vertex
				if (tokens.size()>3 && tokens[0] == "v") positions.push_back(glm::vec3(_stringToFloat(tokens[1]), _stringToFloat(tokens[2]), _stringToFloat(tokens[3])));

				//daca am o normala
				if (tokens.size()>3 && tokens[0] == "vn") normals.push_back(glm::vec3(_stringToFloat(tokens[1]), _stringToFloat(tokens[2]), _stringToFloat(tokens[3])));

				//daca am un texcoord
				if (tokens.size()>2 && tokens[0] == "vt") texcoords.push_back(glm::vec2(_stringToFloat(tokens[1]), _stringToFloat(tokens[2])));

				//daca am o fata (f+ minim 3 indecsi)
				if (tokens.size() >= 4 && tokens[0] == "f"){

					//foloseste primul vertex al fetei pentru a determina formatul fetei (v v/t v//n v/t/n) = (1 2 3 4)
					unsigned int face_format = 0;
					if (tokens[1].find("//") != std::string::npos) face_format = 3;
					_faceTokenize(tokens[1], facetokens);
					if (facetokens.size() == 3) face_format = 4; // vertecsi/texcoords/normale
					else{
						if (facetokens.size() == 2){
							if (face_format != 3) face_format = 2;	//daca nu am vertecsi/normale am vertecsi/texcoords
						}
						else{
							face_format = 1; //doar vertecsi
						}
					}

					//primul index din acest poligon
					unsigned int index_of_first_vertex_of_face = -1;


					for (unsigned int num_token = 1; num_token<tokens.size(); num_token++){
						if (tokens[num_token].at(0) == '#') break;					//comment dupa fata
						_faceTokenize(tokens[num_token], facetokens);
						if (face_format == 1){
							//doar pozitie
							int p_index = _stringToInt(facetokens[0]);
							if (p_index>0) p_index -= 1;								//obj has 1...n indices
							else p_index = positions.size() + p_index;				//index negativ

							vertices.push_back(VertexFormat(positions[p_index].x, positions[p_index].y, positions[p_index].z));
						}
						else if (face_format == 2){
							// pozitie si texcoord
							int p_index = _stringToInt(facetokens[0]);
							if (p_index>0) p_index -= 1;								//obj has 1...n indices
							else p_index = positions.size() + p_index;				//index negativ

							int t_index = _stringToInt(facetokens[1]);
							if (t_index>0) t_index -= 1;								//obj has 1...n indices
							else t_index = texcoords.size() + t_index;				//index negativ

							vertices.push_back(VertexFormat(positions[p_index].x, positions[p_index].y, positions[p_index].z, texcoords[t_index].x, texcoords[t_index].y));
						}
						else if (face_format == 3){
							//pozitie si normala
							int p_index = _stringToInt(facetokens[0]);
							if (p_index>0) p_index -= 1;								//obj has 1...n indices
							else p_index = positions.size() + p_index;				//index negativ

							int n_index = _stringToInt(facetokens[1]);
							if (n_index>0) n_index -= 1;								//obj has 1...n indices
							else n_index = normals.size() + n_index;					//index negativ

							vertices.push_back(VertexFormat(positions[p_index].x, positions[p_index].y, positions[p_index].z, normals[n_index].x, normals[n_index].y, normals[n_index].z));
						}
						else{
							//pozitie normala si texcoord
							int p_index = _stringToInt(facetokens[0]);
							if (p_index>0) p_index -= 1;								//obj has 1...n indices
							else p_index = positions.size() + p_index;				//index negativ

							int t_index = _stringToInt(facetokens[1]);
							if (t_index>0) t_index -= 1;								//obj has 1...n indices
							else t_index = normals.size() + t_index;					//index negativ

							int n_index = _stringToInt(facetokens[2]);
							if (n_index>0) n_index -= 1;								//obj has 1...n indices
							else n_index = normals.size() + n_index;					//index negativ

							vertices.push_back(VertexFormat(positions[p_index].x, positions[p_index].y, positions[p_index].z, normals[n_index].x, normals[n_index].y, normals[n_index].z, texcoords[t_index].x, texcoords[t_index].y));
						}

						//adauga si indecsii
						if (num_token<4){
							if (num_token == 1) index_of_first_vertex_of_face = vertices.size() - 1;
							//doar triunghiuri f 0 1 2 3 (4 indecsi, primul e ocupat de f)
							indices.push_back(vertices.size() - 1);
						}
						else{
							//polygon => triunghi cu ultimul predecesor vertexului nou adaugat si 0 relativ la vertecsi poligon(independent clockwise)
							indices.push_back(index_of_first_vertex_of_face);
							indices.push_back(vertices.size() - 2);
							indices.push_back(vertices.size() - 1);
						}
					}//end for
				}//end face

			}//end while
		}
	};
}
