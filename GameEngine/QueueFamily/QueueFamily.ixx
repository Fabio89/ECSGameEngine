export module Engine.Render.Core:QueueFamily;
import vulkan_hpp;
import std;
import <cstdint>;

export enum class QueueFamilyType : size_t
{
	Graphics,
	Present,
	Transfer,
	INDEX_TYPE_COUNT
};

export class QueueFamilyIndices
{
	std::vector<std::optional<uint32_t>> m_families;

public:
	QueueFamilyIndices();

	int size() const;

	const std::optional<uint32_t>& get(QueueFamilyType type) const;
	std::optional<uint32_t>& get(QueueFamilyType type);

	using Iterator = decltype(m_families)::iterator;
	Iterator begin() { return m_families.begin(); }
	Iterator end() { return m_families.end(); }

	using ConstIterator = decltype(m_families)::const_iterator;
	ConstIterator begin() const { return m_families.cbegin(); }
	ConstIterator end() const { return m_families.cend(); }
};

namespace QueueFamilyUtils
{
	export bool areAllIndicesSet(const QueueFamilyIndices&);
	export QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
}

QueueFamilyIndices::QueueFamilyIndices()
	: m_families(static_cast<size_t>(QueueFamilyType::INDEX_TYPE_COUNT))
{}

int QueueFamilyIndices::size() const
{
	return static_cast<int>(m_families.size());
}

const std::optional<uint32_t>& QueueFamilyIndices::get(QueueFamilyType type) const
{
	return m_families.at(static_cast<size_t>(type));
}

std::optional<uint32_t>& QueueFamilyIndices::get(QueueFamilyType type)
{
	return m_families.at(static_cast<size_t>(type));
}

bool QueueFamilyUtils::areAllIndicesSet(const QueueFamilyIndices& indices)
{
	return std::ranges::all_of(indices, [](auto&& index) { return index.has_value(); });
}

QueueFamilyIndices QueueFamilyUtils::findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	QueueFamilyIndices indices;

	std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		const vk::QueueFamilyProperties& family = queueFamilies[i];
		if ((family.queueFlags & vk::QueueFlagBits::eGraphics))
			indices.get(QueueFamilyType::Graphics) = i;
		else if ((family.queueFlags & vk::QueueFlagBits::eTransfer))
			indices.get(QueueFamilyType::Transfer) = i;

		{
			if (device.getSurfaceSupportKHR(i, surface))
				indices.get(QueueFamilyType::Present) = i;
		}

		if (areAllIndicesSet(indices))
			break;
	}

	auto isGraphicsFamily = [](const vk::QueueFamilyProperties& family) { return !!(family.queueFlags & vk::QueueFlagBits::eGraphics); };
	if (auto it = std::ranges::find_if(queueFamilies, isGraphicsFamily); it != queueFamilies.end())
	{
		indices.get(QueueFamilyType::Graphics) = static_cast<uint32_t>(it - queueFamilies.begin());
	}

	return indices;
}