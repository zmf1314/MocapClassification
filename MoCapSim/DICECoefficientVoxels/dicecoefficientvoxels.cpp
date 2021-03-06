﻿/*
    Copyright (C) 2019  Miroslav Krajíček

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "dicecoefficientvoxels.h"

#include <QtConcurrent/QtConcurrentMap>
#include <QTime>

DICECoefficientVoxels::DICECoefficientVoxels(QObject *parent) : QObject(parent)
{
    connect(&m_descWatcher,&QFutureWatcher<cimg_library::CImg<float>>::finished,this, &DICECoefficientVoxels::onComputeDescFinished);
}

void DICECoefficientVoxels::setAnimations(QVector<MocapAnimation *> animations)
{
    m_anims = animations;
    m_distanceMatrix = cv::Mat(m_anims.size(),m_anims.size(),CV_32FC1, cv::Scalar(std::numeric_limits<float>::max()));
}

QFuture<void> DICECoefficientVoxels::computeDescriptors()
{
    const QFuture<QPair<int,cimg_library::CImg<uint8_t>>> future = QtConcurrent::mapped(m_anims,&DICECoefficientVoxels::computeVoxels);
    m_descWatcher.setFuture(future);
    return future;
}

QFuture<void> DICECoefficientVoxels::computeAllDistances()
{
    std::function<void(const MocapAnimation*)> f = std::bind(&DICECoefficientVoxels::computeDistToAll,std::placeholders::_1, m_distanceMatrix, m_anims, m_descriptors);
    const QFuture<void> future = QtConcurrent::map(m_anims, f);
    m_distanceWatcher.setFuture(future);
    return future;
}

QWidget* DICECoefficientVoxels::getVisualization()
{
    return m_vis;
}

void DICECoefficientVoxels::selectionAdded(int animId)
{
    if (!m_descriptors.contains(animId)) return;

    m_visImgs.insert(animId,m_descriptors[animId]);
    refreshVis();
}

void DICECoefficientVoxels::selectionRemoved(int animId)
{
    m_visImgs.remove(animId);
    refreshVis();
}

void DICECoefficientVoxels::onComputeDescFinished()
{
    const auto future = m_descWatcher.future();

    std::for_each(future.constBegin(),future.constEnd(),
                  [this](auto &it){m_descriptors[it.first] = it.second;});
}

float DICECoefficientVoxels::computeDist(const cimg_library::CImg<uint8_t> &desc1,
                                         const cimg_library::CImg<uint8_t> &desc2)
{
    cimg_library::CImg<int> tp = desc1.get_min(desc2);
    cimg_library::CImg<int> fp = desc1 - tp;
    cimg_library::CImg<int> fn = desc2 - tp;

    auto tpHist = tp.get_histogram(2,0,1);
    auto fpHist = fp.get_histogram(2,0,1);
    auto fnHist = fn.get_histogram(2,0,1);

    float dice = 2.0f*static_cast<float>(tpHist(1))/static_cast<float>((2*tpHist(1) + fpHist(1) + fnHist(1)));

    if(dice < 0.001f)
    {
        return std::numeric_limits<float>::max();
    }

    return 1.0f/dice;
}

void DICECoefficientVoxels::computeDistToAll(const MocapAnimation *anim,cv::Mat &reducedResults,QVector<MocapAnimation*> anims, QHash<int,cimg_library::CImg<uint8_t>> descriptors)
{
    const int id = anim->getId();
    auto it = anims.begin();

    while (id > (*it)->getId())
    {
        const int id2 = (*it)->getId();
        reducedResults.at<float>(id,id2) =
                reducedResults.at<float>(id2,id) = computeDist(descriptors[(*it)->getId()],descriptors[anim->getId()]);
        ++it;
    }
}

QPair<int,cimg_library::CImg<uint8_t>> DICECoefficientVoxels::computeVoxels(const MocapAnimation * const anim)
{
    const int color = 1;
    const int numberOfNodes = 31;

    cimg_library::CImg<uint8_t> m_voxelMap = cimg_library::CImg<uint8_t>(20,20,20,1,0);

    for (int n = 0; n < numberOfNodes; ++n)
    {
        for (int f = 1; f < anim->frames(); ++f)
        {
            const cv::Vec3f p = (*anim)(n,f);
            const cv::Vec3f &pos = ((*anim)(n,f) + cv::Vec3f(20.0f,20.0f,20.0f)) * 5.0f/10.0f;
            const cv::Vec3f &prevPos = ((*anim)(n,f-1) + cv::Vec3f(20.0f,20.0f,20.0f)) * 5.0f/10.0f;

            const cv::Vec<uint,3> &posi = cv::Vec3i(std::nearbyintf(pos[0]),std::nearbyintf(pos[1]),std::nearbyintf(pos[2]));
            const cv::Vec<uint,3> &prevPosi = cv::Vec3i(std::nearbyintf(prevPos[0]),std::nearbyintf(prevPos[1]),std::nearbyintf(prevPos[2]));

            if (prevPosi == posi)
            {
                m_voxelMap(posi[0],posi[1],posi[2]) = 1;
            }
            else
            {
                m_voxelMap.draw_line(prevPosi[0],prevPosi[1],prevPosi[2],posi[0],posi[1],posi[2],&color);
            }
        }
    }

    return {anim->getId(),m_voxelMap};
}

void DICECoefficientVoxels::refreshVis()
{
    m_vis->update(m_visImgs);
}
