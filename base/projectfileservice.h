// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef PROJECTFILESERVICE_H
#define PROJECTFILESERVICE_H

#include <QDomDocument>
#include <QString>

class TextureProject;

/// @brief Identifies the stage at which a project load or save operation failed.
enum class ProjectFileError {
   /// @brief The operation completed successfully.
   None,
   /// @brief The input file could not be opened.
   InputOpen,
   /// @brief The input could not be parsed as XML.
   XmlParse,
   /// @brief The parsed document does not describe a valid project graph.
   Validation,
   /// @brief A validated document could not be converted into a project.
   Construction,
   /// @brief The output path exists and overwriting was disabled.
   OutputExists,
   /// @brief The output file could not be opened.
   OutputOpen,
   /// @brief The complete project document could not be written.
   OutputWrite,
   /// @brief The temporary output could not be committed atomically.
   OutputCommit
};

/// @brief Reports the outcome of a project load or save operation.
struct ProjectFileResult {
   /// @brief Operation status code.
   ProjectFileError error = ProjectFileError::None;
   /// @brief Human-readable failure description, or an empty string on success.
   QString message;
   /// @brief Returns whether the operation completed successfully.
   [[nodiscard]] bool succeeded() const noexcept { return error == ProjectFileError::None; }
   /// @brief Converts the result to true when the operation succeeded.
   explicit operator bool() const noexcept { return succeeded(); }
};

/// @brief Provides shared, non-GUI project loading and saving operations.
class ProjectFileService final {
public:
   /// @brief Loads and validates a project from an XML file.
   /// @param path Path of the project file to read.
   /// @param project Project that receives the loaded graph.
   /// @return The operation result, including an error message on failure.
   [[nodiscard]] static ProjectFileResult load(const QString& path, TextureProject& project);

   /// @brief Saves a project atomically as an XML file.
   /// @param path Destination path for the project file.
   /// @param project Project to serialize and mark as saved after a successful commit.
   /// @param overwrite Whether an existing destination may be replaced.
   /// @return The operation result, including an error message on failure.
   [[nodiscard]] static ProjectFileResult save(const QString& path, TextureProject& project,
                                               bool overwrite);

private:
   /// @brief Constructs a project from a parsed and validated XML document.
   /// @param document Parsed project document.
   /// @param project Project that receives the loaded graph.
   /// @param path Path used to identify the project in validation errors.
   /// @return The operation result, including construction failures.
   [[nodiscard]] static ProjectFileResult loadDocument(const QDomDocument& document,
                                                       TextureProject& project,
                                                       const QString& path);

   /// @brief Validates project structure, generators, sources, and graph acyclicity.
   /// @param document Parsed project document to validate.
   /// @param project Project providing the available generator registry.
   /// @param path Path used to identify the project in validation errors.
   /// @return Success when the document is valid, otherwise a validation failure.
   [[nodiscard]] static ProjectFileResult validate(const QDomDocument& document,
                                                   const TextureProject& project,
                                                   const QString& path);
};

#endif  // PROJECTFILESERVICE_H
