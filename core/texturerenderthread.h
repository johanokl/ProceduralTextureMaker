/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTURERENDERTHREAD_H
#define TEXTURERENDERTHREAD_H

#include "texturenode.h"
#include <QSize>

class TextureProject;

/**
 * @brief The TextureRenderThread class
 *
 * Class for moving the image rendering to a separate CPU thread.
 * Stays idle when no image needs to be generated.
 * One thread only handles one image size, so create one instance for each size.
 */
class TextureRenderThread : public QObject
{
   Q_OBJECT

public:
   TextureRenderThread(const QSize renderSize, QMap<int, TextureNodePtr> nodes);
   ~TextureRenderThread() override = default;
   void abort();

public slots:
   void imageUpdated();
   void nodeRemoved(int remNode);
   void nodeAdded(const TextureNodePtr& newNode);

private:
   void generate();
   bool noneGenerated;
   const QSize renderSize;
   QMap<int, TextureNodePtr> nodes;
   bool aborted;
};

#endif // TEXTURERENDERTHREAD_H
